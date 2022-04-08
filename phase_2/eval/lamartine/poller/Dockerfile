FROM ruby:2.6
RUN mkdir /poller
COPY . /poller
WORKDIR /poller
RUN bundle
CMD ["./poller.rb"]
