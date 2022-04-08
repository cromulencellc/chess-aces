const FileSystem = require('./fs.js')

const nodePath = require('path')

const UNIX_SEP_REGEX = /\//g;
const WIN_SEP_REGEX = /\\/g;

class AnonymousFiles extends FileSystem {
    constructor(connection, {root, cwd}) {
        let opts = {root: nodePath.join(root, '_anonymous'),
                    cwd: cwd}
        
        super(connection, opts);
        
      }

      
}

module.exports = AnonymousFiles