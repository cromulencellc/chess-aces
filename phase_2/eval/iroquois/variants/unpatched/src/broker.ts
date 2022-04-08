import { connect, AsyncMqttClient,
  Packet, IPublishPacket
 } from 'async-mqtt'

import Logger from './logger'

import { Message } from './models/message'
import { Subscription } from './models/subscription'
import { Topic } from './models/topic'

export class Broker {
  mqtt: AsyncMqttClient

  constructor(mqtt_url: string) {
    this.mqtt = connect(mqtt_url)

    this.mqtt.on('connect', this.did_connect.bind(this))
    this.mqtt.on('message', this.did_receive_message.bind(this))
    this.mqtt.on('error', this.did_error.bind(this))
  }

  publish(topic: number | string | Topic | undefined,
          message: string | Buffer):
  Promise<IPublishPacket> {
    if (undefined == topic) {
      throw "Tried to publish to `undefined` topic"
    }
    if (topic instanceof Topic) return this.publish(topic.name, message)
    if ('number' == typeof topic) {
      return this.publish(Topic.find(topic), message)
    }

    return this.mqtt.publish(topic, message)
  }

  did_connect(packet: Packet) {
    Logger.INFO('connected to MQTT')

    let topic_names = Topic.allNames()

    this.bulk_subscribe(topic_names)
  }

  subscribe(topic_name: string): void {
    this.mqtt.subscribe([topic_name]).then(_grants => {
      Logger.INFO(`subscribed to ${topic_name}`)
    })
  }

  bulk_subscribe(topic_names: string[]): void {
    this.mqtt.subscribe(topic_names).then(grants => {
      Logger.INFO(`subscribed to ${grants.length}/${topic_names.length} topics`)
    })
  }

  unsubscribe(topic_name: string): void {
    this.mqtt.unsubscribe([topic_name]).then(_grants => {
      Logger.INFO(`unsubscribed from ${topic_name}`)
    })
  }

  bulk_unsubscribe(topic_names: string[]): void {
    this.mqtt.unsubscribe(topic_names).then(grants => {
       Logger.INFO(`unsubscribed from ${grants.length}/${topic_names.length} topics`)
    })
  }

  did_error(error: Error) {
    Logger.ERROR(`got MQTT error ${error.name}\n${error.message}`)
  }

  did_receive_message(topic_name: string, payload: Buffer, packet: Packet) {
    Logger.INFO(`message for ${topic_name}`)

    let topic = new Topic({name: topic_name}).save() // will find or create, w/e

    Message.create({topic: topic, content: payload})
  }
}
