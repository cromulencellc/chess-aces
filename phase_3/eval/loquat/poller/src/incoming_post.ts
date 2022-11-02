import { IncomingMessage } from "http"

export default class IncomingPost {
  message: IncomingMessage
  body: Buffer[] = []
}