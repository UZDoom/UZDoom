#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <wayland-client.hpp>
#include <wayland-client-protocol-extra.hpp>

struct xdg_wm_dialog_v1;
struct xdg_dialog_v1;

namespace wayland
{
class xdg_wm_dialog_v1_t;
enum class xdg_wm_dialog_v1_error : uint32_t;
class xdg_dialog_v1_t;

namespace detail
{
  extern const wl_interface xdg_wm_dialog_v1_interface;
  extern const wl_interface xdg_dialog_v1_interface;
}

/** \brief create dialogs related to other toplevels

      The xdg_wm_dialog_v1 interface is exposed as a global object allowing
      to register surfaces with a xdg_toplevel role as "dialogs" relative to
      another toplevel.

      The compositor may let this relation influence how the surface is
      placed, displayed or interacted with.

      Warning! The protocol described in this file is currently in the testing
      phase. Backward compatible changes may be added together with the
      corresponding interface version bump. Backward incompatible changes can
      only be done by creating a new major version of the extension.
    
*/
class xdg_wm_dialog_v1_t : public proxy_t
{
private:
  struct events_t : public detail::events_base_t
  {
  };

  static int dispatcher(uint32_t opcode, const std::vector<detail::any>& args, const std::shared_ptr<detail::events_base_t>& e);

  xdg_wm_dialog_v1_t(proxy_t const &wrapped_proxy, construct_proxy_wrapper_tag /*unused*/);

public:
  xdg_wm_dialog_v1_t();
  explicit xdg_wm_dialog_v1_t(const proxy_t &proxy);
  xdg_wm_dialog_v1_t(xdg_wm_dialog_v1 *p, wrapper_type t = wrapper_type::standard);

  xdg_wm_dialog_v1_t proxy_create_wrapper();

  static const std::string interface_name;

  operator xdg_wm_dialog_v1*() const;

  /** \brief create a dialog object
      \param toplevel 

        Creates a xdg_dialog_v1 object for the given toplevel. See the interface
        description for more details.

	Compositors must raise an already_used error if clients attempt to
	create multiple xdg_dialog_v1 objects for the same xdg_toplevel.
      
  */
  xdg_dialog_v1_t get_xdg_dialog(xdg_toplevel_t const& toplevel);

  /** \brief Minimum protocol version required for the \ref get_xdg_dialog function
  */
  static constexpr std::uint32_t get_xdg_dialog_since_version = 1;

};

/** \brief 

  */
enum class xdg_wm_dialog_v1_error : uint32_t
  {
  /** \brief the xdg_toplevel object has already been used to create a xdg_dialog_v1 */
  already_used = 0
};


/** \brief dialog object

      A xdg_dialog_v1 object is an ancillary object tied to a xdg_toplevel. Its
      purpose is hinting the compositor that the toplevel is a "dialog" (e.g. a
      temporary window) relative to another toplevel (see
      xdg_toplevel.set_parent). If the xdg_toplevel is destroyed, the xdg_dialog_v1
      becomes inert.

      Through this object, the client may provide additional hints about
      the purpose of the secondary toplevel. This interface has no effect
      on toplevels that are not attached to a parent toplevel.
    
*/
class xdg_dialog_v1_t : public proxy_t
{
private:
  struct events_t : public detail::events_base_t
  {
  };

  static int dispatcher(uint32_t opcode, const std::vector<detail::any>& args, const std::shared_ptr<detail::events_base_t>& e);

  xdg_dialog_v1_t(proxy_t const &wrapped_proxy, construct_proxy_wrapper_tag /*unused*/);

public:
  xdg_dialog_v1_t();
  explicit xdg_dialog_v1_t(const proxy_t &proxy);
  xdg_dialog_v1_t(xdg_dialog_v1 *p, wrapper_type t = wrapper_type::standard);

  xdg_dialog_v1_t proxy_create_wrapper();

  static const std::string interface_name;

  operator xdg_dialog_v1*() const;

  /** \brief mark dialog as modal

        Hints that the dialog has "modal" behavior. Modal dialogs typically
        require to be fully addressed by the user (i.e. closed) before resuming
        interaction with the parent toplevel, and may require a distinct
        presentation.

        Clients must implement the logic to filter events in the parent
        toplevel on their own.

        Compositors may choose any policy in event delivery to the parent
        toplevel, from delivering all events unfiltered to using them for
        internal consumption.
      
  */
  void set_modal();

  /** \brief Minimum protocol version required for the \ref set_modal function
  */
  static constexpr std::uint32_t set_modal_since_version = 1;

  /** \brief mark dialog as not modal

        Drops the hint that this dialog has "modal" behavior. See
        xdg_dialog_v1.set_modal for more details.
      
  */
  void unset_modal();

  /** \brief Minimum protocol version required for the \ref unset_modal function
  */
  static constexpr std::uint32_t unset_modal_since_version = 1;

};



}
