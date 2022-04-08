const _ = require('lodash');
const nodePath = require('path');
const uuid = require('uuid');
const Promise = require('bluebird');
const {createReadStream, createWriteStream, constants} = require('fs');
const fsAsync = require('./helpers/fs-async');
const errors = require('./errors');

const UNIX_SEP_REGEX = /\//g;
const WIN_SEP_REGEX = /\\/g;

class FileSystem {
  constructor(connection, {root, cwd} = {}) {
    this.connection = connection;
    this.cwd = nodePath.normalize((cwd || '/').replace(WIN_SEP_REGEX, '/'));
    this._root = nodePath.resolve(root || process.cwd());
  }

  get root() {
    return this._root;
  }

  _resolvePath(path = '.') {
    let resolvedPath = path.replace(WIN_SEP_REGEX, '/');

    let joinedPath = nodePath.
        join(this.root, this.cwd, resolvedPath).
        replace(UNIX_SEP_REGEX, nodePath.sep).
        replace(WIN_SEP_REGEX, nodePath.sep)

    let fsPath = nodePath.normalize(joinedPath)    
    let clientPath = joinedPath.replace(WIN_SEP_REGEX, '/');

    return {
      clientPath,
      fsPath
    };
  }

  currentDirectory() {
    return this.cwd;
  }

  get(fileName) {
    const {fsPath} = this._resolvePath(fileName);
    return fsAsync.stat(fsPath)
    .then((stat) => _.set(stat, 'name', fileName));
  }

  list(path = '.') {
    const {fsPath} = this._resolvePath(path);
    return fsAsync.readdir(fsPath)
    .then((fileNames) => {
      return Promise.map(fileNames, (fileName) => {
        const filePath = nodePath.join(fsPath, fileName);
        return fsAsync.access(filePath, constants.F_OK)
        .then(() => {
          return fsAsync.stat(filePath)
          .then((stat) => _.set(stat, 'name', fileName));
        })
        .catch(() => null);
      });
    })
    .then(_.compact);
  }

  chdir(path = '.') {
    const resolvedPath = path.replace(WIN_SEP_REGEX, '/');

    const joinedPath = nodePath.isAbsolute(resolvedPath)
      ? nodePath.normalize(resolvedPath)
      : nodePath.join('/', this.cwd, resolvedPath);

    const fsPath = nodePath.resolve(nodePath.join(this.root, joinedPath)
      .replace(UNIX_SEP_REGEX, nodePath.sep)
      .replace(WIN_SEP_REGEX, nodePath.sep));

    const clientPath = joinedPath.replace(WIN_SEP_REGEX, '/');
    return fsAsync.stat(fsPath)
    .tap((stat) => {
      if (!stat.isDirectory()) throw new errors.FileSystemError('Not a valid directory');
    })
    .then(() => {
      this.cwd = clientPath;
      return this.currentDirectory();
    });
  }

  write(fileName, {append = false, start = undefined} = {}) {
    const {fsPath, clientPath} = this._resolvePath(fileName);
    const stream = createWriteStream(fsPath, {flags: !append ? 'w+' : 'a+', start});
    stream.once('error', () => fsAsync.unlink(fsPath));
    stream.once('close', () => stream.end());
    return {
      stream,
      clientPath
    };
  }

  read(fileName, {start = undefined} = {}) {
    const {fsPath, clientPath} = this._resolvePath(fileName);
    return fsAsync.stat(fsPath)
    .tap((stat) => {
      if (stat.isDirectory()) throw new errors.FileSystemError('Cannot read a directory');
    })
    .then(() => {
      const stream = createReadStream(fsPath, {flags: 'r', start});
      return {
        stream,
        clientPath
      };
    });
  }

  delete(path) {
    const {fsPath} = this._resolvePath(path);
    return fsAsync.stat(fsPath)
    .then((stat) => {
      if (stat.isDirectory()) return fsAsync.rmdir(fsPath);
      else return fsAsync.unlink(fsPath);
    });
  }

  mkdir(path) {
    const {fsPath} = this._resolvePath(path);
    return fsAsync.mkdir(fsPath)
    .then(() => fsPath);
  }

  rename(from, to) {
    const {fsPath: fromPath} = this._resolvePath(from);
    const {fsPath: toPath} = this._resolvePath(to);
    return fsAsync.rename(fromPath, toPath);
  }

  chmod(path, mode) {
    const {fsPath} = this._resolvePath(path);
    return fsAsync.chmod(fsPath, mode);
  }

  getUniqueName() {
    return uuid.v4().replace(/\W/g, '');
  }
}
module.exports = FileSystem;
