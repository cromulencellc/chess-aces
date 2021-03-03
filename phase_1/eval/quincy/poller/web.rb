require 'sinatra/base'

# this global variable is a hash of variables that are used to generate variable
# HTML documents
ViewLocals = {
  body_class: 'default',

  body_data_key: 'default',
  body_data_value: 'default',

  h1: 'default',
  first_para: 'hey',
  second_para: 'hi',

  list_item_count: 5
}

# the top-level `Web` class is a Sinatra "modular application"
# it's managed by the `Poller` class
class Web < Sinatra::Application
  # a GET request to `/` renders the `views/index.haml` file into HTML
  get '/' do
    haml :index, locals: ViewLocals
  end
end
