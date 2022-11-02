// start the weather data collection
const plugin = require('./plugins/tempest');

const database = require('./models/database')


const fsPromises = require('fs/promises')

import * as express from 'express'
import { AddressInfo } from 'net';
var session = require('express-session');

if (! process.env.CHESS) {
    console.log("[FATAL] This application is for research purposes only")
    process.exit(1)
}


var app = express();

app.use(session({resave: true, saveUninitialized: true, secret: 'XCR3rsasa%RDHHH', cookie: { maxAge: 60000 }}));

app.use(express.urlencoded({extended:false}));
app.use(express.json());

// setup the website routes
var weather_routes = require('./routes/weather');
var admin_routes = require('./routes/admin');

app.use('/', admin_routes);
app.use('/', weather_routes);

var web_server = app.listen(parseInt(process.env.HTTP_PORT || '8080'), 
function() {
    let addr_info = web_server.address() as AddressInfo
    console.log(`web server is listening on port ${addr_info.port}`)

});

let admin_data_path = process.env.ADMIN_DATA_PATH || '/data/admin.json'
fsPromises.readFile(admin_data_path, {encoding: 'utf8'}).
then((admin_data: String) => {
    let decoded = JSON.parse(admin_data.toString())
    return database.create_admin_user(decoded.userid, decoded.password_digest)
}).
then((_create_result: any) => {
    return fsPromises.unlink(admin_data_path)
}).
then((_unlink_result: any) => console.log(`loaded and deleted ${admin_data_path}`)).
catch((e: Error) => {
    console.log(`got ${e.message} loading admin data, assuming it's unnecessary`)
})