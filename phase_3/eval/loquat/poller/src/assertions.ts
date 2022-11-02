import { isArray } from '@tool-belt/type-predicates'
import logger from './logger'
import util from 'util'

export var assertion_count: number = 0

export function assert_array_match(expected, got) {
  assertion_count++

  if (!isArray(expected)) {
    throw `assert_array_match first arg (expected) should be array, got ${util.inspect(expected)}`
  }

  if (!isArray(got)) {
    logger.ERROR("expected:")
    console.log(expected)
    logger.ERROR("got non-array:")
    console.log(got)

    throw "failed assert_array_match (got non-array)"
  }

  if (expected.length != got.length) {
    logger.ERROR("expected:")
    console.log(expected)
    logger.ERROR("got:")
    console.log(got)

    throw "failed assert_array_match (length mismatch)"
  }

  for (var i = 0; i < expected.length; i++) {
    if (expected[i] != got[i]) {
      logger.ERROR("expected:")
      console.log(expected)
      logger.ERROR("got:")
      console.log(got)

      throw `failed assert_array_match (index ${i})`
    }
  }
}

export function assert_equal(expected, got) {
  if (isArray(expected)) {
    return assert_array_match(expected, got)
  }

  assertion_count++

  if (expected == got) return

  logger.ERROR("expected:")
  console.log(expected)
  logger.ERROR("got:")
  console.log(got)

  throw "failed assert_equal"
}

export function assert(expr) {
  assertion_count++
  if (expr) return

  throw "failed assert"
}

export function refute(expr) {
  assertion_count++
  if (! expr) return

  throw "failed refute"
}