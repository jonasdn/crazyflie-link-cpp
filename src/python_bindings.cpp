#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/numpy.h>

#include "native_link/Packet.hpp"
#include "native_link/Connection.h"
// #include "USBManager.h"

// USBManager* g_usbMgr;

PYBIND11_MAKE_OPAQUE(std::array<uint8_t, CRTP_MAXSIZE>)

namespace py = pybind11;
using namespace pybind11::literals;

template<class T>
std::string toString(const T& x) 
{
  std::stringstream sstr;
  sstr << x;
  return sstr.str();
}

PYBIND11_MODULE(nativelink, m) {

  // Helper for Packet
  // py::bind_vector<std::array<uint8_t, CRTP_MAXSIZE>>(m, "ByteArray");
  // g_usbMgr = new USBManager();

  // Packet
  py::class_<Packet>(m, "Packet", py::buffer_protocol())
      .def(py::init<>())
      .def_property("channel", &Packet::channel, &Packet::setChannel)
      .def_property("port", &Packet::port, &Packet::setPort)
      // .def_property(
      //     "data", [](Packet &p) -> py::array {
      //       auto dtype = py::dtype(py::format_descriptor<uint8_t>::format());
      //       auto base = py::array(dtype, {p.size()}, {sizeof(uint8_t)});
      //       return py::array(
      //           dtype, {p.size()}, {sizeof(uint8_t)}, p.data(), base); }, [](Packet &) {})
      .def_buffer([](Packet &p) -> py::buffer_info {
        return py::buffer_info(p.payload(), p.payloadSize(), /*readonly*/ false);
      })
      .def("__len__", [](const Packet &p) {
        return p.payloadSize();
      })
      .def("__getitem__", [](const Packet &p, py::ssize_t i) {
        if (i >= (long int)p.payloadSize())
          throw py::index_error();
        return p.payload()[i];
      })
      .def("__setitem__", [](Packet &p, py::ssize_t i, int v) {
        if (i >= (long int)p.payloadSize())
          throw py::index_error();
        if (v < 0 || v > 255)
          throw py::value_error();
        return p.payload()[i] = v;
      })
      .def_property("payload",
        [](const Packet &p) -> py::bytes {
          return py::bytes(reinterpret_cast<const char*>(p.payload()), p.payloadSize());
        },
        [](Packet &p, py::bytes value) {
          char *buffer;
          ssize_t length;
          PYBIND11_BYTES_AS_STRING_AND_SIZE(value.ptr(), &buffer, &length);
          p.setPayloadSize(length);
          std::memcpy(p.payload(), buffer, length);
        })
      .def_property("size", &Packet::payloadSize, &Packet::setPayloadSize)
      .def_property_readonly("valid", &Packet::valid)
      .def("__repr__", &toString<Packet>);

  //
  py::class_<Connection::Statistics>(m, "ConnectionStatistics")
      .def_readonly("sent_count", &Connection::Statistics::sent_count)
      .def("__repr__", &toString<Connection::Statistics>);

  // Connection
  py::class_<Connection>(m, "Connection")
      .def(py::init<
           const std::string &>())
      .def_static("scan", &Connection::scan)
      .def("send", &Connection::send)
      .def("recv", &Connection::recv, py::arg("timeout"))
      .def_property_readonly("uri", &Connection::uri)
      .def_property_readonly("statistics", &Connection::statistics)
      .def("__repr__", &toString<Connection>);
}
