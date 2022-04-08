"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.Broker = void 0;
const async_mqtt_1 = require("async-mqtt");
const logger_1 = __importDefault(require("./logger"));
const message_1 = require("./models/message");
const topic_1 = require("./models/topic");
class Broker {
    constructor(mqtt_url) {
        this.mqtt = async_mqtt_1.connect(mqtt_url);
        this.mqtt.on('connect', this.did_connect.bind(this));
        this.mqtt.on('message', this.did_receive_message.bind(this));
        this.mqtt.on('error', this.did_error.bind(this));
    }
    publish(topic, message) {
        if (undefined == topic) {
            throw "Tried to publish to `undefined` topic";
        }
        if (topic instanceof topic_1.Topic)
            return this.publish(topic.name, message);
        if ('number' == typeof topic) {
            return this.publish(topic_1.Topic.find(topic), message);
        }
        return this.mqtt.publish(topic, message);
    }
    did_connect(packet) {
        logger_1.default.INFO('connected to MQTT');
        let topic_names = topic_1.Topic.allNames();
        this.bulk_subscribe(topic_names);
    }
    subscribe(topic_name) {
        this.mqtt.subscribe([topic_name]).then(_grants => {
            logger_1.default.INFO(`subscribed to ${topic_name}`);
        });
    }
    bulk_subscribe(topic_names) {
        this.mqtt.subscribe(topic_names).then(grants => {
            logger_1.default.INFO(`subscribed to ${grants.length}/${topic_names.length} topics`);
        });
    }
    unsubscribe(topic_name) {
        this.mqtt.unsubscribe([topic_name]).then(_grants => {
            logger_1.default.INFO(`unsubscribed from ${topic_name}`);
        });
    }
    bulk_unsubscribe(topic_names) {
        this.mqtt.unsubscribe(topic_names).then(grants => {
            logger_1.default.INFO(`unsubscribed from ${grants.length}/${topic_names.length} topics`);
        });
    }
    did_error(error) {
        logger_1.default.ERROR(`got MQTT error ${error.name}\n${error.message}`);
    }
    did_receive_message(topic_name, payload, packet) {
        logger_1.default.INFO(`message for ${topic_name}`);
        let topic = new topic_1.Topic({ name: topic_name }).save(); // will find or create, w/e
        message_1.Message.create({ topic: topic, content: payload });
    }
}
exports.Broker = Broker;
