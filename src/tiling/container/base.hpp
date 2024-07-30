#pragma once
#include <memory>
#include <tiling/focus.hpp>
#include <tiling/state.hpp>
#include <utility>
#include <vector>

#include "wlr/util/box.h"

class GfServer;
struct GfToplevel;

// TODO: Remove
enum gfwl_container_type {
  GFWL_CONTAINER_UNKNOWN  = 0,
  GFWL_CONTAINER_HSPLIT   = 1,
  GFWL_CONTAINER_VSPLIT   = 2,
  GFWL_CONTAINER_TOPLEVEL = 3,
  GFWL_CONTAINER_ROOT     = 4,
};

class GfContainer : public std::enable_shared_from_this<GfContainer> {
  friend class GfContainerRoot;
  friend class GfContainerToplevel;
  friend class GfContainerSplit;

public:
  /* This will be used for non root containers in theory. */
  explicit GfContainer(GfServer&                    server,
                       std::weak_ptr<GfContainer>   parent_container,
                       const gfwl_container_type    e_type,
                       std::weak_ptr<GfTilingState> tiling_state) :
      e_type(e_type), parent_container(std::move(parent_container)),
      tiling_state(std::move(tiling_state)), server(server) {};

  /* I have this constructor without a parent container for root containers. */
  explicit GfContainer(GfServer&                    server,
                       const gfwl_container_type    e_type,
                       std::weak_ptr<GfTilingState> tiling_state) :
      e_type(e_type), tiling_state(std::move(tiling_state)), server(server) {};

  virtual std::weak_ptr<GfContainer>      insert(GfToplevel* to_insert);
  virtual void                            parse_containers();
  virtual void                            close();
  void                                    set_focused_toplevel_container();

  const wlr_box&                          get_box();
  std::vector<std::weak_ptr<GfContainer>> get_top_level_container_list();
  gfwl_split_direction       get_split_dir_from_container_type() const;
  gfwl_split_direction       get_split_dir_longer() const;

  void                       split_containers();

  const gfwl_container_type  e_type = GFWL_CONTAINER_UNKNOWN;

  std::weak_ptr<GfContainer> parent_container;
  std::vector<std::shared_ptr<GfContainer>> child_containers;
  std::weak_ptr<GfTilingState>              tiling_state;

  GfServer&                                 server;

private:
  void         vert_split_containers();
  void         hori_split_containers();
  virtual void set_container_box(struct wlr_box box);

  void         move_container_to(const std::weak_ptr<GfContainer>& new_parent);
  std::weak_ptr<GfContainer> insert_child(GfToplevel* to_insert);

  std::weak_ptr<GfContainer> insert_based_on_longer_dir(GfToplevel* to_insert);

  std::weak_ptr<GfContainer>
  insert_child_in_split(GfToplevel*              to_insert,
                        enum gfwl_container_type split_container_type);

  std::weak_ptr<GfContainer>
          insert_child_in_split(GfToplevel*                       to_insert,
                                const std::weak_ptr<GfContainer>& insert_after,
                                enum gfwl_container_type          split_container_type);
  void    parse_children();

  wlr_box box = {.x = 0, .y = 0, .width = 0, .height = 0};
};
