var mongoose = require('mongoose');

var schema = mongoose.Schema;

//The Schema for the Data to be Stored
var AdminSchema = schema({
    userid: String,
    password: String,
});

var AdminModel = mongoose.model('AdminData', AdminSchema);

exports.AdminAccessModel = AdminModel;