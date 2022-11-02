const config = require('../config/config');
const mongoose = require('mongoose');
const fsPromises = require('fs/promises');
mongoose.connect(process.env.MONGO_URL || "mongodb://mongodb:27017/weatherdb");
var db2 = require('../models/database');
import * as express from 'express';
let router = express.Router();
var token_data = 'token not loaded yet';
fsPromises.readFile('/token', { encoding: 'utf8' }).
    then((token) => token_data = token.replace("\n", "")).
    catch((err) => token_data = `error reading /token ${err}`);
var admin_panel = function (_req, res) {
    var device_results = db2.get_last_device_status();
    var results;
    if (Object.keys(device_results).length == 0) {
        results = { message: 'There is no admin data available', result: 'success' };
    }
    else {
        results = device_results;
    }
    res.send(results);
};
var auth = function (req, res, next) {
    if (req.session.token) {
        console.log("this user is good to go");
        next();
    }
    else {
        console.log("this user isn't authenticated");
        res.send({ message: 'you need to login', result: 'not authenticated' });
    }
};
var admin_login = function (_req, res) {
    console.log("user needs to login");
    res.send({ message: 'you need to login', result: 'not authenticated' });
};
var admin_login_post = async function (req, res) {
    const AdminModels = require('../models/admin');
    const AdminAccessModel = AdminModels.AdminAccessModel;
    const md5 = require('md5');
    var hash;
    try {
        hash = md5(req.body.password);
    }
    catch {
        console.log("Bad password value sent");
        res.redirect('/admin/login');
        return;
    }
    var admins = await AdminAccessModel.findOne({ 'userid': req.body.userid });
    if (admins == undefined) {
        console.log("unsuccessful login");
        res.redirect('/admin/login');
        return;
    }
    if (admins.password == hash) {
        console.log("this is a successful login");
        req.session.token = 'alpha';
        res.send({ message: 'login successful',
            result: 'authenticated',
            token: token_data });
    }
    else {
        console.log("unsuccessful login");
        res.redirect('/admin/login');
    }
};
var admin_delete_all = async function (_req, res) {
    console.log("remove all data");
    await mongoose.connect(process.env.MONGO_URL || "mongodb://mongodb:27017/weatherdb");
    var db = mongoose.connection;
    var collections = db.collections;
    for (let i in collections) {
        // don't let the admin auth table get deleted
        if (i != 'admindatas') {
            console.log(`dropping ${i}`);
            try {
                await db.dropCollection(i);
            }
            catch {
            }
        }
    }
    res.send({ message: 'weather data deleted', result: 'success' });
};
router.get('/admin', auth, admin_panel);
router.get('/admin/login', admin_login);
router.post('/admin/login', admin_login_post);
router.post('/admin/delete_all', auth, admin_delete_all);
module.exports = router;
