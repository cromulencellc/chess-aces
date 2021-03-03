FROM ruby:2.6-alpine
RUN mkdir /douglas-pov
COPY . /douglas-pov
WORKDIR /douglas-pov
RUN bundle
CMD ["./douglas-pov.rb"]
