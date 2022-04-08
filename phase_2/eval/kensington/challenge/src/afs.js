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
}

module.exports = AnonymousFiles