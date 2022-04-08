#!/usr/bin/env ts-node-script

import {Block, Assignment} from
'./udmf_classes'
import {Vertex, Sector, Sidedef, DrawStyle, Linedef} from './map_classes'

import { Pwad } from './pwad'

import { DefaultMap } from './default_map'

import Logger from './logger'

import xml from 'xml'
import { XmlObject } from 'xml'

import fs from 'fs'
import process, { exit } from 'process'
import util from 'util'
import { TooBigError } from './too_big_error'
import { SvgWriter } from './svg_writer'


let pwad_path = process.argv[2]
let pwad = new Pwad(pwad_path)

let svg_path = process.argv[3]
let writer = new SvgWriter(svg_path, pwad.map)
writer.write()
