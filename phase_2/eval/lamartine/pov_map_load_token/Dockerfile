FROM ruby:2.7
RUN mkdir /pov
COPY . /pov
WORKDIR /pov
RUN bundle
CMD ["./pov.rb"]
