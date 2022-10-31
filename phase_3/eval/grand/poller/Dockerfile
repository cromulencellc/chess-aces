FROM ruby:2.6
RUN mkdir /poller
COPY . /poller
ADD ./base_data /data
WORKDIR /poller
RUN bundle
RUN chmod +x ./poller.rb
CMD ["./poller.rb"]
