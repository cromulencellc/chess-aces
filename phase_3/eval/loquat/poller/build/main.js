"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const crypto_1 = __importDefault(require("crypto"));
const dgram_1 = __importDefault(require("dgram"));
const http_1 = __importDefault(require("http"));
const process_1 = __importDefault(require("process"));
const url_1 = require("url");
const util_1 = __importDefault(require("util"));
const random_1 = __importDefault(require("random"));
const seedrandom_1 = __importDefault(require("seedrandom"));
const sprintf_1 = require("sprintf");
const ws_1 = __importDefault(require("ws"));
const needle_1 = __importDefault(require("needle"));
const assertions_1 = require("./assertions");
const logger_1 = __importDefault(require("./logger"));
const navstar_1 = __importDefault(require("./navstar"));
const user_1 = __importDefault(require("./user"));
const timing_1 = __importDefault(require("./timing"));
const type_predicates_1 = require("@tool-belt/type-predicates");
const tlm_proto_1 = require("./tlm_proto");
class Poller {
    constructor() {
        this.ws_urls = new Map();
        this.navstars = new Map();
        this.users = [];
        this.time_step = 1000 * 60 * 60; // 1000ms * 60s * 60m; hour steps
        this.timings = new Map();
        this.assertion_count = 0;
        this.ACTIVITIES = ("homepage " +
            //"ping_ws " + 
            "submit_tlm_per submit_tlm_firehose " +
            "submit_tlm_firehose_nada " +
            // "check_tlm " +
            // "check_noexist " +
            "filter_tlm " +
            "create_user add_user_satellite remove_user_satellite " +
            // "user_watch_allow user_watch_forbid " +
            "check_admin_login check_user_refuse_admin ").split(/\s+/);
        this.rejected_activities = [];
        this.got_message_sym = Symbol('got_message_sym');
        this.timed_out_sym = Symbol('timed_out_sym');
        this.host = process_1.default.env['HOST'] || 'ta3_loquat';
        this.http_port = parseInt(process_1.default.env['HTTP_PORT'] || '3080');
        this.tlm_port = parseInt(process_1.default.env['TLM_PORT'] || '3081');
        this.expected_token = process_1.default.env['EXPECTED_TOKEN'];
        var seed = process_1.default.env['SEED'] ||
            crypto_1.default.randomInt(2 ** 48 - 1).toString();
        console.log(`SEED=${seed}`);
        this.rng = random_1.default.clone((0, seedrandom_1.default)(seed));
        this.rng.patch();
        var len = process_1.default.env['LENGTH'] ||
            this.rng.int(50, 100).toString();
        console.log(`LENGTH=${len}`);
        this.length = parseInt(len);
        this.timeout = 1000 * parseInt(process_1.default.env['TIMEOUT'] || '5');
        this.current_time = new Date(Date.now() - (this.length * this.time_step));
        this.base_url = new url_1.URL(`http://${this.host}:${this.http_port}/`);
        this.navstar_promise = navstar_1.default.load_navstars().then(ns => this.navstars = ns);
        this.admin_username = process_1.default.env['ADMIN_USERNAME'];
        this.admin_password = process_1.default.env['ADMIN_PASSWORD'];
        if (!this.admin_username || !this.admin_password) {
            throw "expected ADMIN_USERNAME and ADMIN_PASSWORD environment variables";
        }
        for (let a of this.ACTIVITIES) {
            if (this[a] instanceof Function) {
                this.timings.set(a, new timing_1.default());
            }
            else {
                this.rejected_activities.push(a);
                logger_1.default.INFO(`rejected activity ${a}`);
            }
        }
        this.udp_sock = dgram_1.default.createSocket('udp4');
    }
    async run() {
        await this.navstar_promise;
        if (0 == this.navstars.size) {
            throw "expected navstars to be loaded";
        }
        if (0 == this.timings.size) {
            logger_1.default.ERROR("no activities, that was easy");
            return;
        }
        let known_activity_names = Array.from(this.timings.keys());
        await this.load_ws_urls();
        await this.check_admin_login();
        for (let i = 0; i < this.length; i++) {
            let acti_idx = this.rng.int(0, this.timings.size - 1);
            let acti_name = known_activity_names[acti_idx];
            let acti_fn = this[acti_name].bind(this);
            let acti_time = this.timings.get(acti_name);
            if (!(acti_fn instanceof Function)) {
                throw `couldn't get activity named ${acti_name} (got a ${typeof acti_fn})`;
            }
            logger_1.default.INFO(`trying ${acti_name}`);
            let timeout = setTimeout(() => {
                logger_1.default.FATAL(`${this.timeout}ms timeout expired`);
                process_1.default.exit(1);
            }, this.timeout);
            await acti_time.time(() => acti_fn());
            clearTimeout(timeout);
        }
    }
    print_report() {
        if (0 != this.rejected_activities.length) {
            console.log('rejected activities: (not implemented?)');
            console.log(this.rejected_activities.join("\n"));
        }
        console.log((0, sprintf_1.sprintf)("%20s\t%s\t%s\t%s\t%s", 'activity', 'count', 'min', 'avg', 'max'));
        this.timings.forEach((tmg, name) => {
            console.log((0, sprintf_1.sprintf)("%20s\t%d\t%1.4f\t%1.4f\t%1.4f", name, tmg.count, tmg.min, tmg.avg(), tmg.max));
        });
        console.log(`${assertions_1.assertion_count} assertions`);
    }
    //
    // ACTIVITIES
    //
    async homepage() {
        let got = await this.get('/');
        (0, assertions_1.assert_equal)(200, got.statusCode);
    }
    async ping_ws() {
        let url = this.ws_urls.get('ping');
        let ping_ws = new ws_1.default(url);
        let ping_ws_open = await new Promise((resolve, _reject) => {
            ping_ws.on('open', () => resolve(ping_ws));
        });
        logger_1.default.DEBUG(ping_ws_open);
        logger_1.default.DEBUG(`did ${url} open`);
        await new Promise((resolve, _reject) => {
            ping_ws.on('message', data => {
                logger_1.default.DEBUG(data);
                resolve(null);
            });
        });
        logger_1.default.DEBUG(`websocket opened`);
        let got_px = new Promise((resolve, _reject) => {
            ping_ws.on('message', data => resolve(data));
        });
        logger_1.default.DEBUG('sending ping');
        ping_ws.send('ping');
        logger_1.default.DEBUG('waiting for pong');
        let got = await got_px;
        (0, assertions_1.assert_equal)("pong", got.toString());
    }
    async submit_tlm_firehose() {
        let user = await this.random_user_with_navstar();
        let navstar = this.navstars.get(user.random_navstar_id(this.rng));
        let mesg = navstar.telemetryAt(this.next_timestamp());
        let firehose_url = this.ws_urls.get('firehose');
        firehose_url.username = user.name;
        firehose_url.password = user.password;
        logger_1.default.DEBUG(firehose_url);
        let firehose_ws = new ws_1.default(firehose_url);
        await new Promise((resolve, _reject) => {
            firehose_ws.on('open', () => resolve(firehose_ws));
        });
        let sids = await new Promise((resolve, _reject) => {
            firehose_ws.on('message', data => resolve(data.toString()));
        });
        let sida = JSON.parse(sids);
        (0, type_predicates_1.assertIsArray)(sida);
        (0, assertions_1.assert_equal)(user.navstar_ids.sort(), sida.sort());
        logger_1.default.DEBUG('sending tlm');
        await new Promise((resolve, reject) => {
            this.udp_sock.send(mesg, this.tlm_port, this.host, (err, sent_bytes) => {
                if (err)
                    return reject(err);
                resolve(sent_bytes);
            });
        });
        logger_1.default.DEBUG('waiting for message');
        let got = await new Promise((resolve, _reject) => {
            firehose_ws.on('message', data => resolve(data));
        });
        let recv_blob = JSON.parse(got.toString());
        (0, assertions_1.assert_equal)(navstar.uuid, recv_blob['satellite_id']);
    }
    async submit_tlm_per() {
        let user = await this.random_user_with_navstar();
        let navstar = this.navstars.get(user.random_navstar_id(this.rng));
        logger_1.default.DEBUG(user.navstar_ids);
        logger_1.default.DEBUG(navstar);
        let mesg = navstar.telemetryAt(this.next_timestamp());
        let per_url = new url_1.URL(this.ws_urls.get('per-satellite').toString());
        per_url.username = user.name;
        per_url.password = user.password;
        per_url.pathname =
            per_url.pathname.replace(':id', navstar.uuid);
        logger_1.default.DEBUG(per_url);
        let per_ws = new ws_1.default(per_url);
        await new Promise((resolve, _reject) => {
            per_ws.on('open', () => resolve(per_ws));
        });
        let sids = await new Promise((resolve, _reject) => {
            per_ws.on('message', data => resolve(data.toString()));
        });
        let sida = JSON.parse(sids);
        (0, type_predicates_1.assertIsArray)(sida);
        (0, assertions_1.assert_equal)([navstar.uuid], sida);
        logger_1.default.DEBUG('subscribed to expected satellite');
        logger_1.default.DEBUG(user.id);
        logger_1.default.DEBUG('sending tlm');
        await new Promise((resolve, reject) => {
            this.udp_sock.send(mesg, this.tlm_port, this.host, (err, sent_bytes) => {
                if (err)
                    return reject(err);
                resolve(sent_bytes);
            });
        });
        logger_1.default.DEBUG('waiting for message');
        let got = await new Promise((resolve, _reject) => {
            per_ws.on('message', data => {
                // logger.DEBUG(util.inspect(data))
                resolve(data);
            });
        });
        // logger.DEBUG(`got ${got}`)
        let recv_blob = JSON.parse(got.toString());
        (0, assertions_1.assert_equal)(navstar.uuid, recv_blob['satellite_id']);
    }
    async submit_tlm_firehose_nada() {
        await this.create_user();
        let user = this.users[this.users.length - 1];
        let navstar = this.random_navstar();
        let mesg = navstar.telemetryAt(this.next_timestamp());
        let firehose_url = this.ws_urls.get('firehose');
        firehose_url.username = user.name;
        firehose_url.password = user.password;
        let firehose_ws = new ws_1.default(firehose_url);
        await new Promise((resolve, _reject) => {
            firehose_ws.on('open', () => resolve(firehose_ws));
        });
        let sids = await new Promise((resolve, _reject) => {
            firehose_ws.on('message', data => resolve(data.toString()));
        });
        let sida = JSON.parse(sids);
        (0, type_predicates_1.assertIsArray)(sida);
        (0, assertions_1.assert_equal)(user.navstar_ids.sort(), sida.sort());
        let maybe_got_message = new Promise((resolve, _reject) => {
            firehose_ws.on('message', data => resolve(this.got_message_sym));
        });
        let maybe_timeout = new Promise((resolve, _reject) => {
            setTimeout(() => {
                resolve(this.timed_out_sym);
            }, this.timeout / 2);
        });
        logger_1.default.DEBUG('sending tlm');
        await new Promise((resolve, reject) => {
            this.udp_sock.send(mesg, this.tlm_port, this.host, (err, sent_bytes) => {
                if (err)
                    return reject(err);
                resolve(sent_bytes);
            });
        });
        logger_1.default.DEBUG('waiting for no message');
        await Promise.
            any([maybe_got_message, maybe_timeout]).
            then(result => {
            (0, assertions_1.assert_equal)(this.timed_out_sym, result);
        });
    }
    async filter_tlm() {
        let user = await this.random_user_with_navstar();
        let navstar = this.navstars.get(user.random_navstar_id(this.rng));
        // create filter
        let filter_battery_gteq = this.rng.boolean();
        let battery_level = 0.25 + (this.rng.normal()() / 2);
        let ts = this.next_timestamp();
        let metric = navstar.telemetryContentAt(ts);
        var will_match = false;
        var filter = {};
        if (filter_battery_gteq) {
            filter['bt_level_gt'] = battery_level;
            will_match = metric.bt.level >= battery_level;
        }
        else {
            filter['bt_level_lt'] = battery_level;
            will_match = metric.bt.level < battery_level;
        }
        filter['satellite_ids'] = [navstar.uuid];
        let filter_post_got = await this.post('/filter', filter);
        (0, assertions_1.assert_equal)(201, filter_post_got.statusCode);
        (0, assertions_1.assert)(filter_post_got.headers['location']);
        let filter_path = filter_post_got.headers['location'];
        // submit tlm
        logger_1.default.DEBUG('sending tlm');
        await new Promise((resolve, reject) => {
            this.udp_sock.send(tlm_proto_1.Telemetry.encode(metric).finish(), this.tlm_port, this.host, (err, sent_bytes) => {
                if (err)
                    return reject(err);
                resolve(sent_bytes);
            });
        });
        logger_1.default.DEBUG(`expecting match: ${will_match}`);
        await new Promise((resolve, _reject) => {
            setTimeout(resolve, 1000);
        });
        // witness me
        let search_got = await this.user_get(filter_path, user);
        (0, assertions_1.assert_equal)(200, search_got.statusCode);
        var got_data = JSON.parse(search_got.read());
        var did_find = false;
        if (!(0, type_predicates_1.isArray)(got_data))
            got_data = [got_data];
        logger_1.default.DEBUG(metric);
        for (let datum of got_data) {
            if (Date.parse(datum['tx_at']) == ts.valueOf()) {
                logger_1.default.DEBUG(datum);
                did_find = true;
                break;
            }
        }
        (0, assertions_1.assert_equal)(will_match, did_find);
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
    async remove_user_satellite() {
        let u = await this.random_user_with_navstar();
        let n_id = u.remove_random_navstar_id(this.rng);
        let got = await this.admin_delete(`/admin/user/${u.name}/satellites/${n_id}`);
        (0, assertions_1.assert_equal)(204, got.statusCode);
        let check_got = await this.admin_get(`/admin/user/${u.name}`);
        (0, assertions_1.assert_equal)(200, check_got.statusCode);
        let body = JSON.parse(check_got.read());
        (0, assertions_1.assert_array_match)(u.navstar_ids.sort(), body['satellite_ids'].sort());
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
        if ((0, type_predicates_1.isUndefined)(this.expected_token))
            return;
        let got_token = got.
            read().
            toString().
            replace(/\s+$/, '');
        logger_1.default.DEBUG(util_1.default.inspect(this.expected_token));
        logger_1.default.DEBUG(util_1.default.inspect(got_token));
        (0, assertions_1.assert_equal)(this.expected_token, got_token);
    }
    async check_user_refuse_admin() {
        let got = await this.get('/admin');
        (0, assertions_1.assert_equal)(401, got.statusCode);
        if ((0, type_predicates_1.isUndefined)(this.expected_token))
            return;
        let got_content = got.read();
        if (!got_content) {
            (0, assertions_1.assert)(true);
            return;
        }
        logger_1.default.DEBUG(`kinda wanted ${got_content} to be empty :/`);
        (0, assertions_1.refute)(got_content.toString().match(this.expected_token));
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
    async user_get(path, user) {
        let url = new url_1.URL(path, this.base_url);
        logger_1.default.DEBUG(`GET user ${user.name} ${url}`);
        return new Promise((resolve, reject) => {
            http_1.default.get(url, { auth: `${user.name}:${user.password}` }, resp => {
                resolve(resp);
            }).
                on('error', e => {
                logger_1.default.FATAL(`user GET ${url} got ${e}`);
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
        let navstar_key = navstar_keys[this.rng.integer(0, navstar_keys.length - 1)];
        return this.navstars.get(navstar_key);
    }
    async random_user() {
        if (0 == this.users.length) {
            logger_1.default.INFO("no known users, adding one");
            await this.create_user();
        }
        let idx = this.rng.integer(0, this.users.length - 1);
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
        let idx = this.rng.integer(0, users_with_navstars.length - 1);
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
    let p = new Poller();
    await p.run();
    p.print_report();
    console.log("Poller exited successfully");
    process_1.default.exit(0);
})();
