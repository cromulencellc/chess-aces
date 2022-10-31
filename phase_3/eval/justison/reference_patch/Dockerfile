FROM chessexternal.apogee-research.com:5010/apogee_ta5/testbed_release/cs_base_phase_2:2021_06_02
#FROM registry.mlb.cromulence.com/chess/cs_base/phase_3:2022_03_10

RUN apt-get update -y
RUN apt-get install -y libgd-dev  
RUN apt-get install -y libxml2-dev
RUN apt-get install -y libxslt1-dev
RUN apt-get install -y libgeoip-dev

RUN echo chess:chess | chpasswd

ADD ./ /home/challenge
ADD ./base_data/ /usr/local/nginx/

RUN mkdir /service
RUN chown -R chess:chess /service

ENV PORT=8000
ENV HOST=challenge_container
ENV CHESS=Chess_System

RUN echo -n 09vee78cnw0mjw3bLOiPPmyn15mwmlks > /token
RUN chmod 644 /token

WORKDIR /home/challenge/nginx-e2d07e4ec636
RUN ./auto/configure --with-http_ssl_module --with-http_v2_module --with-http_realip_module --with-http_addition_module --with-http_xslt_module --with-http_image_filter_module --with-http_geoip_module --with-http_sub_module --with-http_dav_module --with-http_flv_module --with-http_mp4_module --with-http_gunzip_module --with-http_gzip_static_module --with-http_auth_request_module --with-http_random_index_module --with-http_secure_link_module --with-http_degradation_module --with-http_slice_module --with-http_stub_status_module
RUN make 

CMD ./objs/nginx
