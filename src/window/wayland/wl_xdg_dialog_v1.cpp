#include "wl_xdg_dialog_v1.hpp"

using namespace wayland;
using namespace wayland::detail;

const wl_interface* xdg_wm_dialog_v1_interface_destroy_request[0] = {
};

const wl_interface* xdg_wm_dialog_v1_interface_get_xdg_dialog_request[2] = {
  &xdg_dialog_v1_interface,
  &xdg_toplevel_interface,
};

const wl_message xdg_wm_dialog_v1_interface_requests[2] = {
  {
    "destroy",
    "",
    xdg_wm_dialog_v1_interface_destroy_request,
  },
  {
    "get_xdg_dialog",
    "no",
    xdg_wm_dialog_v1_interface_get_xdg_dialog_request,
  },
};

const wl_message xdg_wm_dialog_v1_interface_events[0] = {
};

const wl_interface wayland::detail::xdg_wm_dialog_v1_interface =
  {
    "xdg_wm_dialog_v1",
    1,
    2,
    xdg_wm_dialog_v1_interface_requests,
    0,
    xdg_wm_dialog_v1_interface_events,
  };

const wl_interface* xdg_dialog_v1_interface_destroy_request[0] = {
};

const wl_interface* xdg_dialog_v1_interface_set_modal_request[0] = {
};

const wl_interface* xdg_dialog_v1_interface_unset_modal_request[0] = {
};

const wl_message xdg_dialog_v1_interface_requests[3] = {
  {
    "destroy",
    "",
    xdg_dialog_v1_interface_destroy_request,
  },
  {
    "set_modal",
    "",
    xdg_dialog_v1_interface_set_modal_request,
  },
  {
    "unset_modal",
    "",
    xdg_dialog_v1_interface_unset_modal_request,
  },
};

const wl_message xdg_dialog_v1_interface_events[0] = {
};

const wl_interface wayland::detail::xdg_dialog_v1_interface =
  {
    "xdg_dialog_v1",
    1,
    3,
    xdg_dialog_v1_interface_requests,
    0,
    xdg_dialog_v1_interface_events,
  };

xdg_wm_dialog_v1_t::xdg_wm_dialog_v1_t(const proxy_t &p)
  : proxy_t(p)
{
  if(proxy_has_object() && get_wrapper_type() == wrapper_type::standard)
    {
      set_events(std::shared_ptr<detail::events_base_t>(new events_t), dispatcher);
      set_destroy_opcode(0U);
    }
  set_interface(&xdg_wm_dialog_v1_interface);
  set_copy_constructor([] (const proxy_t &p) -> proxy_t
    { return xdg_wm_dialog_v1_t(p); });
}

xdg_wm_dialog_v1_t::xdg_wm_dialog_v1_t()
{
  set_interface(&xdg_wm_dialog_v1_interface);
  set_copy_constructor([] (const proxy_t &p) -> proxy_t
    { return xdg_wm_dialog_v1_t(p); });
}

xdg_wm_dialog_v1_t::xdg_wm_dialog_v1_t(xdg_wm_dialog_v1 *p, wrapper_type t)
  : proxy_t(reinterpret_cast<wl_proxy*> (p), t){
  if(proxy_has_object() && get_wrapper_type() == wrapper_type::standard)
    {
      set_events(std::shared_ptr<detail::events_base_t>(new events_t), dispatcher);
      set_destroy_opcode(0U);
    }
  set_interface(&xdg_wm_dialog_v1_interface);
  set_copy_constructor([] (const proxy_t &p) -> proxy_t
    { return xdg_wm_dialog_v1_t(p); });
}

xdg_wm_dialog_v1_t::xdg_wm_dialog_v1_t(proxy_t const &wrapped_proxy, construct_proxy_wrapper_tag /*unused*/)
  : proxy_t(wrapped_proxy, construct_proxy_wrapper_tag()){
  set_interface(&xdg_wm_dialog_v1_interface);
  set_copy_constructor([] (const proxy_t &p) -> proxy_t
    { return xdg_wm_dialog_v1_t(p); });
}

xdg_wm_dialog_v1_t xdg_wm_dialog_v1_t::proxy_create_wrapper()
{
  return {*this, construct_proxy_wrapper_tag()};
}

const std::string xdg_wm_dialog_v1_t::interface_name = "xdg_wm_dialog_v1";

xdg_wm_dialog_v1_t::operator xdg_wm_dialog_v1*() const
{
  return reinterpret_cast<xdg_wm_dialog_v1*> (c_ptr());
}

xdg_dialog_v1_t xdg_wm_dialog_v1_t::get_xdg_dialog(xdg_toplevel_t const& toplevel)
{
  proxy_t p = marshal_constructor(1U, &xdg_dialog_v1_interface, nullptr, toplevel.proxy_has_object() ? reinterpret_cast<wl_object*>(toplevel.c_ptr()) : nullptr);
  return xdg_dialog_v1_t(p);
}


int xdg_wm_dialog_v1_t::dispatcher(uint32_t opcode, const std::vector<any>& args, const std::shared_ptr<detail::events_base_t>& e)
{
  return 0;
}


xdg_dialog_v1_t::xdg_dialog_v1_t(const proxy_t &p)
  : proxy_t(p)
{
  if(proxy_has_object() && get_wrapper_type() == wrapper_type::standard)
    {
      set_events(std::shared_ptr<detail::events_base_t>(new events_t), dispatcher);
      set_destroy_opcode(0U);
    }
  set_interface(&xdg_dialog_v1_interface);
  set_copy_constructor([] (const proxy_t &p) -> proxy_t
    { return xdg_dialog_v1_t(p); });
}

xdg_dialog_v1_t::xdg_dialog_v1_t()
{
  set_interface(&xdg_dialog_v1_interface);
  set_copy_constructor([] (const proxy_t &p) -> proxy_t
    { return xdg_dialog_v1_t(p); });
}

xdg_dialog_v1_t::xdg_dialog_v1_t(xdg_dialog_v1 *p, wrapper_type t)
  : proxy_t(reinterpret_cast<wl_proxy*> (p), t){
  if(proxy_has_object() && get_wrapper_type() == wrapper_type::standard)
    {
      set_events(std::shared_ptr<detail::events_base_t>(new events_t), dispatcher);
      set_destroy_opcode(0U);
    }
  set_interface(&xdg_dialog_v1_interface);
  set_copy_constructor([] (const proxy_t &p) -> proxy_t
    { return xdg_dialog_v1_t(p); });
}

xdg_dialog_v1_t::xdg_dialog_v1_t(proxy_t const &wrapped_proxy, construct_proxy_wrapper_tag /*unused*/)
  : proxy_t(wrapped_proxy, construct_proxy_wrapper_tag()){
  set_interface(&xdg_dialog_v1_interface);
  set_copy_constructor([] (const proxy_t &p) -> proxy_t
    { return xdg_dialog_v1_t(p); });
}

xdg_dialog_v1_t xdg_dialog_v1_t::proxy_create_wrapper()
{
  return {*this, construct_proxy_wrapper_tag()};
}

const std::string xdg_dialog_v1_t::interface_name = "xdg_dialog_v1";

xdg_dialog_v1_t::operator xdg_dialog_v1*() const
{
  return reinterpret_cast<xdg_dialog_v1*> (c_ptr());
}

void xdg_dialog_v1_t::set_modal()
{
  marshal(1U);
}


void xdg_dialog_v1_t::unset_modal()
{
  marshal(2U);
}


int xdg_dialog_v1_t::dispatcher(uint32_t opcode, const std::vector<any>& args, const std::shared_ptr<detail::events_base_t>& e)
{
  return 0;
}


