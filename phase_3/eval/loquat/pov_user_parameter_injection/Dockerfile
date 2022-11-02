FROM ruby:3.1-alpine
RUN mkdir /pov
COPY . /pov
WORKDIR /pov
RUN bundle --local
CMD ["./pov.rb"]
