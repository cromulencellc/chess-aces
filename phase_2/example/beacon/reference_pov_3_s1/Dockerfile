FROM chessinternal.apogee-research.com:5005/apogee/testbed_environment/ruby_2.7
RUN mkdir /pov
COPY . /pov
WORKDIR /pov
RUN bundle install --local
CMD ["ruby", "pov.rb"]
