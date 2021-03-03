#include "connection.hpp"

#include "dataload.hpp"

void Connection::service() {
  while (true) {
    switch (state) {
    case ConnectionState::oqtopus:
      oqto.consume(in_fd, out_fd);
      if (oqto.want_data_load) {
        state = ConnectionState::data;
        oqto.want_data_load = false;
      }
      if (oqto.want_disconnect) {
        state = ConnectionState::disconnect;
      }
      break;
    case ConnectionState::data:
      Dataload::do_dl(in_fd, out_fd);
      state = ConnectionState::oqtopus;
      break;
    case ConnectionState::disconnect:
      return;
    }
  }
}
