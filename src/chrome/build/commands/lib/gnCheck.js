const config = require('./config')
const util = require('./util')

const gnCheck = (buildConfig = config.defaultBuildConfig, options = {}) => {
  config.buildConfig = buildConfig
  config.update(options)
  util.run('gn', ['check', config.outputDir ], config.defaultOptions)
}

module.exports = gnCheck
