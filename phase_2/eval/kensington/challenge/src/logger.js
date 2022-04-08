const process = require('process')
const util = require('util')

const LevelNames = ["_INVALID", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"];

let LogLevels = new Map()

LevelNames.forEach((name, index, _arr) => {LogLevels.set(name, index)})

const log_level = LogLevels.get(process.env['LOG_LEVEL'] || 'DEBUG') || 1

function maybe_log(candidate_level, message) {
  if (candidate_level < log_level) return

  console.log(`[${LevelNames[candidate_level]}] ${util.format(message)}`)
}

module.exports = {
  NOOP: (_message) => {},
  DEBUG: (message) => maybe_log(1, message),
  INFO:  (message) => maybe_log(2, message),
  WARN:  (message) => maybe_log(3, message),
  ERROR: (message) => maybe_log(4, message),
  FATAL: (message) => maybe_log(5, message)
}
