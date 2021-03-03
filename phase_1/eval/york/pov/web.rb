require 'sinatra/base'

class Web < Sinatra::Application
  get '/' do
return <<EOF
<!doctype html>
<html><head><title>cool</title></head>
<body>
<hr>
<marquee class='heyyyyyyyyyyyyy>let's go</marquee>
<hr>
</body>
</html>
EOF
  end
end
