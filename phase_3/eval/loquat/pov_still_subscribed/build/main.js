"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const crypto_1 = require("crypto");
const dgram_1 = __importDefault(require("dgram"));
const http_1 = __importDefault(require("http"));
const process_1 = __importDefault(require("process"));
const url_1 = require("url");
const util_1 = __importDefault(require("util"));
const ws_1 = __importDefault(require("ws"));
const needle_1 = __importDefault(require("needle"));
const assertions_1 = require("./assertions");
const logger_1 = __importDefault(require("./logger"));
const navstar_1 = __importDefault(require("./navstar"));
const user_1 = __importDefault(require("./user"));
class Pov {
    constructor() {
        this.ws_urls = new Map();
        this.navstars = new Map();
        this.users = [];
        this.time_step = 1000 * 60 * 60; // 1000ms * 60s * 60m; hour steps
        this.assertion_count = 0;
        this.got_message_sym = Symbol('got_message_sym');
        this.timed_out_sym = Symbol('timed_out_sym');
        this.host = process_1.default.env['HOST'] || 'ta3_loquat';
        this.http_port = parseInt(process_1.default.env['HTTP_PORT'] || '3080');
        this.tlm_port = parseInt(process_1.default.env['TLM_PORT'] || '3081');
        this.timeout = 1000 * parseInt(process_1.default.env['TIMEOUT'] || '5');
        this.current_time = new Date(Date.now());
        this.base_url = new url_1.URL(`http://${this.host}:${this.http_port}/`);
        this.navstar_promise = navstar_1.default.load_navstars().then(ns => this.navstars = ns);
        this.admin_username = process_1.default.env['ADMIN_USERNAME'];
        this.admin_password = process_1.default.env['ADMIN_PASSWORD'];
        if (!this.admin_username || !this.admin_password) {
            throw "expected ADMIN_USERNAME and ADMIN_PASSWORD environment variables";
        }
        this.udp_sock = dgram_1.default.createSocket('udp4');
    }
    async run() {
        await this.navstar_promise;
        if (0 == this.navstars.size) {
            throw "expected navstars to be loaded";
        }
        await this.load_ws_urls();
        await this.check_admin_login();
        let timeout = setTimeout(() => {
            logger_1.default.FATAL(`${this.timeout}ms timeout expired`);
            process_1.default.exit(0);
        }, this.timeout);
        // set up a user and a navstar they have access to
        let user = await this.random_user_with_navstar();
        let navstar_id = user.random_navstar_id();
        let navstar = this.navstars.get(navstar_id);
        // sock is the user's data feed
        let sock = await this.start_firehose_subscription(user, navstar);
        let got_subscription_confirmation = false;
        await new Promise((resolve, _reject) => {
            sock.on('message', data => {
                if (!got_subscription_confirmation) {
                    console.log(`subscription confirmed ${data.toString()}`);
                    got_subscription_confirmation = true;
                    resolve();
                }
            });
        });
        // watch the user's data feed for a message
        let got_privdata_px = new Promise((resolve, _reject) => {
            sock.on('message', data => {
                let heredoc = (0, crypto_1.randomUUID)();
                console.log(`PRIVDATA_HERE=${heredoc}`);
                console.log(util_1.default.inspect(data.toString()));
                console.log(heredoc);
                resolve();
            });
        });
        // admin removes the user's access
        await this.remove_user_satellite(user, navstar);
        // send data to the ingest port that the user shouldn't get
        await this.send_privileged_data(navstar);
        // the `await` in `main()` downbelow waits for this to return
        return got_privdata_px;
    }
    //
    // ACTIVITIES
    //
    async ping_ws() {
        let ping_ws = new ws_1.default(this.ws_urls.get('ping'));
        await new Promise((resolve, _reject) => {
            ping_ws.on('open', () => resolve(ping_ws));
        });
        let got_px = new Promise((resolve, _reject) => {
            ping_ws.on('message', data => resolve(data));
        });
        ping_ws.send('ping');
        let got = await got_px;
        (0, assertions_1.assert_equal)("pong", got.toString());
    }
    async start_firehose_subscription(user, navstar) {
        let mesg = navstar.telemetryAt(this.next_timestamp());
        let firehose_url = this.ws_urls.get('firehose');
        firehose_url.username = user.name;
        firehose_url.password = user.password;
        let firehose_ws = new ws_1.default(firehose_url);
        await new Promise((resolve, _reject) => {
            firehose_ws.on('open', () => resolve(firehose_ws));
        });
        return firehose_ws;
    }
    async send_privileged_data(navstar) {
        let mesg = navstar.telemetryAt(this.next_timestamp());
        await new Promise((resolve, reject) => {
            this.udp_sock.send(mesg, this.tlm_port, this.host, (err, sent_bytes) => {
                if (err)
                    return reject(err);
                resolve(sent_bytes);
            });
        });
    }
    async create_user() {
        let u = new user_1.default();
        let got = await this.post('/user', u.params());
        (0, assertions_1.assert_equal)(201, got.statusCode);
        u.id = got.headers.location.split('/').pop();
        this.users.push(u);
    }
    async add_user_satellite() {
        let u = await this.random_user();
        let navstar = this.random_navstar();
        let got = await this.admin_post(`/admin/user/${u.name}/satellites`, {
            satellite_id: navstar.uuid
        });
        (0, assertions_1.assert_equal)(204, got.statusCode);
        if (!u.navstar_ids.includes(navstar.uuid)) {
            u.navstar_ids.push(navstar.uuid);
        }
        let check_got = await this.admin_get(`/admin/user/${u.name}`);
        (0, assertions_1.assert_equal)(200, check_got.statusCode);
        let body = JSON.parse(check_got.read());
        (0, assertions_1.assert_equal)(u.navstar_ids.sort(), body['satellite_ids'].sort());
    }
    async remove_user_satellite(u, navstar) {
        let n_id = navstar.uuid;
        let got = await this.admin_delete(`/admin/user/${u.name}/satellites/${n_id}`);
        (0, assertions_1.assert_equal)(204, got.statusCode);
        let check_got = await this.admin_get(`/admin/user/${u.name}`);
        (0, assertions_1.assert_equal)(200, check_got.statusCode);
        let body = JSON.parse(check_got.read());
        (0, assertions_1.assert_array_match)([], body['satellite_ids'].sort());
    }
    //
    // ACTIVITY HELPERS
    //
    async load_ws_urls() {
        let got = await this.get('/ws.json');
        (0, assertions_1.assert_equal)(200, got.statusCode);
        let ws_urls = JSON.parse(got.read());
        for (const url_name in ws_urls) {
            let n = new url_1.URL(this.base_url.toString());
            let b = new url_1.URL(ws_urls[url_name]);
            n.protocol = b.protocol;
            n.pathname = b.pathname;
            this.ws_urls.set(url_name, n);
        }
        // logger.DEBUG(this.ws_urls)
    }
    async check_admin_login() {
        let got = await this.admin_get('/admin');
        (0, assertions_1.assert_equal)(200, got.statusCode);
    }
    async get(path) {
        let url = new url_1.URL(path, this.base_url);
        logger_1.default.DEBUG(`GET ${url}`);
        return new Promise((resolve, reject) => {
            http_1.default.get(url, resp => {
                resolve(resp);
            }).
                on("error", e => {
                logger_1.default.FATAL(`GET ${url} got ${e}`);
                reject(e);
            });
        });
    }
    async admin_get(path) {
        let url = new url_1.URL(path, this.base_url);
        logger_1.default.DEBUG(`GET admin ${url}`);
        return new Promise((resolve, reject) => {
            http_1.default.get(url, { auth: `${this.admin_username}:${this.admin_password}` }, resp => {
                resolve(resp);
            }).
                on('error', e => {
                logger_1.default.FATAL(`admin GET ${url} got ${e}`);
                reject(e);
            });
        });
    }
    async post(path, body_params) {
        let url = new url_1.URL(path, this.base_url);
        logger_1.default.DEBUG(`POST ${url} ${util_1.default.inspect(body_params)}`);
        return await (0, needle_1.default)('post', url.toString(), body_params);
    }
    async admin_post(path, body_params) {
        let url = new url_1.URL(path, this.base_url);
        logger_1.default.DEBUG(`POST admin ${url} ${util_1.default.inspect(body_params)}`);
        return await (0, needle_1.default)('post', url.toString(), body_params, { username: this.admin_username, password: this.admin_password });
    }
    async admin_delete(path, body_params) {
        let url = new url_1.URL(path, this.base_url);
        logger_1.default.DEBUG(`DELETE admin ${url} ${util_1.default.inspect(body_params)}`);
        return await (0, needle_1.default)('delete', url.toString(), body_params, { username: this.admin_username, password: this.admin_password });
    }
    random_navstar() {
        let navstar_keys = Array.from(this.navstars.keys());
        let navstar_key = navstar_keys[(0, crypto_1.randomInt)(navstar_keys.length - 1)];
        return this.navstars.get(navstar_key);
    }
    async random_user() {
        if (0 == this.users.length) {
            logger_1.default.INFO("no known users, adding one");
            await this.create_user();
        }
        if (1 == this.users.length) {
            return this.users[0];
        }
        let idx = (0, crypto_1.randomInt)(this.users.length - 1);
        return this.users[idx];
    }
    async random_user_with_navstar() {
        let users_with_navstars = this.users.filter(u => u.has_navstars());
        if (0 == users_with_navstars.length) {
            logger_1.default.INFO("no users with navstars, adding one");
            let u = await this.random_user();
            let s = this.random_navstar();
            let got = await this.admin_post(`/admin/user/${u.name}/satellites`, {
                satellite_id: s.uuid
            });
            (0, assertions_1.assert_equal)(204, got.statusCode);
            u.navstar_ids.push(s.uuid);
            logger_1.default.DEBUG(u);
            return u;
        }
        if (1 == users_with_navstars.length) {
            return users_with_navstars[0];
        }
        let idx = (0, crypto_1.randomInt)(users_with_navstars.length - 1);
        return users_with_navstars[idx];
    }
    next_timestamp() {
        let cur = this.current_time;
        this.current_time =
            new Date(this.current_time.valueOf() + this.time_step);
        return cur;
    }
}
(async () => {
    let p = new Pov();
    await p.run();
    console.log("pov exited successfully");
    process_1.default.exit(0);
})();
