FROM ruby:2.6-alpine
RUN mkdir /douglas-poller
COPY . /douglas-poller
WORKDIR /douglas-poller
RUN bundle
CMD ["./douglas-poller.rb"]
