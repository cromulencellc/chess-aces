FROM ruby:2.6
RUN mkdir /hamlin-poller
COPY . /hamlin-poller
WORKDIR /hamlin-poller
RUN bundle
CMD ["./poller.rb"]
