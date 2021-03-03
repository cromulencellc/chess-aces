FROM ruby:2.6
RUN mkdir /pov
COPY . /pov
WORKDIR /pov
RUN bundle
CMD ["./pov.rb"]
