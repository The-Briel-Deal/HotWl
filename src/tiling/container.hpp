#pragma once
#include "state.hpp"
#include "wlr/util/box.h"
#include "xdg_shell.hpp"
#include <includes.hpp>
#include <memory>
#include <vector>

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
      e_type(e_type), parent_container(parent_container),
      tiling_state(tiling_state), server(server){};

  /* I have this constructor without a parent container for root containers. */
  explicit GfContainer(GfServer&                    server,
                       const gfwl_container_type    e_type,
                       std::weak_ptr<GfTilingState> tiling_state) :
      e_type(e_type), tiling_state(tiling_state), server(server){};

  virtual std::weak_ptr<GfContainer>        insert(gfwl_toplevel* toplevel);
  virtual void                              parse_containers();
  virtual void                              close();
  void                                      set_focused_toplevel_container();

  const wlr_box&                            get_box();
  std::vector<std::weak_ptr<GfContainer>>   get_top_level_container_list();
  gfwl_split_direction                      get_split_dir_from_container_type();
  gfwl_split_direction                      get_split_dir_longer();

  void                                      split_containers();

  const gfwl_container_type                 e_type = GFWL_CONTAINER_UNKNOWN;

  std::weak_ptr<GfContainer>                parent_container;
  std::vector<std::shared_ptr<GfContainer>> child_containers;
  std::weak_ptr<GfTilingState>              tiling_state;

  GfServer&                                 server;

private:
  void         vert_split_containers();
  void         hori_split_containers();
  virtual void set_container_box(struct wlr_box box);

  void         move_container_to(std::weak_ptr<GfContainer> new_parent);
  std::weak_ptr<GfContainer> insert_child(gfwl_toplevel* toplevel);

  std::weak_ptr<GfContainer>
  insert_based_on_longer_dir(gfwl_toplevel* toplevel);

  std::weak_ptr<GfContainer>
  insert_child_in_split(gfwl_toplevel*           toplevel,
                        enum gfwl_container_type split_container_type);

  std::weak_ptr<GfContainer>
          insert_child_in_split(gfwl_toplevel*             toplevel,
                                std::weak_ptr<GfContainer> insert_after,
                                enum gfwl_container_type   split_container_type);
  void    parse_children();

  wlr_box box = {.x = 0, .y = 0, .width = 0, .height = 0};
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

private:
  void set_to_output_size();
};

class GfContainerToplevel : public GfContainer {
public:
  explicit GfContainerToplevel(gfwl_toplevel* const         toplevel,
                               GfServer&                    server,
                               std::weak_ptr<GfContainer>   parent,
                               std::weak_ptr<GfTilingState> tiling_state) :
      GfContainer(server, parent, GFWL_CONTAINER_TOPLEVEL, tiling_state),
      toplevel(toplevel){};
  ~GfContainerToplevel();

  gfwl_toplevel* const toplevel;

  void                 set_container_box(struct wlr_box box_in);
};

class GfContainerSplit : public GfContainer {
public:
  explicit GfContainerSplit(GfServer&                    server,
                            std::weak_ptr<GfContainer>   parent,
                            const gfwl_container_type    e_type,
                            std::weak_ptr<GfTilingState> tiling_state) :
      GfContainer(server, parent, e_type, tiling_state){};
};

void set_focused_toplevel_container(std::weak_ptr<GfContainer> container);
