#pragma once
#include "state.hpp"
#include "wlr/util/box.h"
#include "xdg_shell.hpp"
#include <includes.hpp>
#include <memory>
#include <variant>
#include <vector>

enum gfwl_container_type {
  GFWL_CONTAINER_UNKNOWN  = 0,
  GFWL_CONTAINER_HSPLIT   = 1,
  GFWL_CONTAINER_VSPLIT   = 2,
  GFWL_CONTAINER_TOPLEVEL = 3,
  GFWL_CONTAINER_ROOT     = 4,
};

class GfContainerRoot;
class GfContainerToplevel;
class GfContainerSplit;

class GfContainer : public std::enable_shared_from_this<GfContainer> {
  friend GfContainerRoot;
  friend GfContainerToplevel;
  friend GfContainerSplit;

public:
  explicit GfContainer(GfServer&                    server,
                       const gfwl_container_type    e_type,
                       std::weak_ptr<GfTilingState> tiling_state) :
      e_type(e_type), tiling_state(tiling_state), server(server){};

  virtual std::weak_ptr<GfContainer> insert(gfwl_toplevel* toplevel);
  virtual void                       parse_containers();
  virtual void                       close() = 0;
  void                               set_focused_toplevel_container();

  const wlr_box&                     get_box();
  gfwl_split_direction               get_split_dir_from_container_type();
  gfwl_split_direction               get_split_dir_longer();

  const gfwl_container_type          e_type = GFWL_CONTAINER_UNKNOWN;

  std::weak_ptr<GfTilingState>       tiling_state;

  GfServer&                          server;

private:
  /* Sets the size and position of a container based on a wlr_box. */
  virtual void set_container_box(struct wlr_box box);
  /* The Dimensions of a container. */

  /* Position Manipulation */

  std::weak_ptr<GfContainer>
          insert_based_on_longer_dir(gfwl_toplevel* toplevel);

  wlr_box box = {.x = 0, .y = 0, .width = 0, .height = 0};
};

class GfContainerToplevel : public GfContainer {
public:
  explicit GfContainerToplevel(gfwl_toplevel* const            toplevel,
                               GfServer&                       server,
                               std::weak_ptr<GfContainerSplit> parent,
                               std::weak_ptr<GfTilingState>    tiling_state) :
      GfContainer(server, GFWL_CONTAINER_TOPLEVEL, tiling_state),
      toplevel(toplevel), parent_container(parent){};
  ~GfContainerToplevel();

  gfwl_toplevel* const            toplevel;

  std::weak_ptr<GfContainerSplit> parent_container;

  void                            close();
  void                            set_container_box(struct wlr_box box_in);

  std::weak_ptr<GfContainer>      insert_sibling(gfwl_toplevel* toplevel);
  std::weak_ptr<GfContainerSplit>
       insert_based_on_longer_dir(gfwl_toplevel* toplevel);

  void move_container_to(std::weak_ptr<GfContainerSplit> new_parent);

  auto find_in_parent();
};

class GfContainerRoot : public GfContainer {
public:
  explicit GfContainerRoot(
      GfServer& server,
      const gfwl_container_type
          e_type, /* TODO: Replace with always being GFWL_CONTAINER_ROOT */
      std::weak_ptr<GfTilingState> tiling_state) :
      GfContainer(server, e_type, tiling_state){};

  std::weak_ptr<GfContainer> insert(gfwl_toplevel* toplevel);
  void                       parse_containers();
  void                       close();

  std::weak_ptr<GfContainerToplevel>
  insert_child_in_split(gfwl_toplevel*             toplevel,
                        std::weak_ptr<GfContainer> insert_after,
                        enum gfwl_container_type   split_container_type);
  std::vector<std::weak_ptr<GfContainerToplevel>>
                                                 get_top_level_container_list();

  std::vector<std::shared_ptr<GfContainerSplit>> child_containers;

private:
  void set_to_output_size();
};

class GfContainerSplit : public GfContainer {
public:
  explicit GfContainerSplit(GfServer&                    server,
                            std::weak_ptr<GfContainer>   parent,
                            const gfwl_container_type    e_type,
                            std::weak_ptr<GfTilingState> tiling_state) :
      GfContainer(server, e_type, tiling_state), parent_container(parent){};

  std::weak_ptr<GfContainerToplevel>
       insert_child_in_split(gfwl_toplevel*                     toplevel,
                             std::weak_ptr<GfContainerToplevel> insert_after,
                             gfwl_container_type split_container_type);

  void parse_containers();

  std::weak_ptr<GfContainerToplevel> insert_child(gfwl_toplevel* toplevel);
  std::weak_ptr<GfContainer>
                             insert_child(gfwl_toplevel*                     toplevel,
                                          std::weak_ptr<GfContainerToplevel> insert_before);

  std::weak_ptr<GfContainer> parent_container;

  std::vector<std::variant<std::shared_ptr<GfContainerSplit>,
                           std::shared_ptr<GfContainerToplevel>>>
      child_containers;

  // private:
  void vert_split_containers();
  void hori_split_containers();
  void split_containers();
  void parse_children();
};

void set_focused_toplevel_container(std::weak_ptr<GfContainer> container);
