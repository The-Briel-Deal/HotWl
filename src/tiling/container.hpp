#pragma once
#include "state.hpp"
#include "xdg_shell.hpp"
#include <includes.hpp>
#include <memory>
#include <vector>

enum gfwl_container_type {
  GFWL_CONTAINER_UNKNOWN = 0,
  GFWL_CONTAINER_HSPLIT = 1,
  GFWL_CONTAINER_VSPLIT = 2,
  GFWL_CONTAINER_TOPLEVEL = 3,
  GFWL_CONTAINER_ROOT = 4,
};

class GfContainer : public std::enable_shared_from_this<GfContainer> {
public:
  /* This will be used for non root containers in theory. */
  explicit GfContainer(gfwl_toplevel *const toplevel, const gfwl_server &server,
                       std::weak_ptr<GfContainer> parent_container,
                       const gfwl_container_type e_type,
                       std::weak_ptr<GfTilingState> tiling_state,
                       const bool is_root)
      : toplevel(toplevel), server(server), parent_container(parent_container),
        e_type(e_type), tiling_state(tiling_state), is_root(is_root){};

  /* I have this constructor without a parent container for root containers. */
  explicit GfContainer(gfwl_toplevel *const toplevel, const gfwl_server &server,
                       const gfwl_container_type e_type,
                       std::weak_ptr<GfTilingState> tiling_state,
                       const bool is_root)
      : toplevel(toplevel), server(server), e_type(e_type),
        tiling_state(tiling_state), is_root(is_root){};
  /* Member Functions */
  void move_container_to(std::weak_ptr<GfContainer> new_parent);

  std::weak_ptr<GfContainer> insert_sibling(gfwl_toplevel *toplevel);
  /* TODO: I think I want to make these private and only make insert public. */
  std::weak_ptr<GfContainer> insert_child(gfwl_toplevel *toplevel);

  std::weak_ptr<GfContainer>
  insert_based_on_longer_dir(gfwl_toplevel *toplevel);

  std::weak_ptr<GfContainer>
  insert_child_in_split(gfwl_toplevel *toplevel,
                        enum gfwl_container_type split_container_type);

  std::weak_ptr<GfContainer>
  insert_child_in_split(gfwl_toplevel *toplevel,
                        std::weak_ptr<GfContainer> insert_after,
                        enum gfwl_container_type split_container_type);

  void set_focused_toplevel_container();

  std::vector<std::weak_ptr<GfContainer>> get_top_level_container_list();
  gfwl_split_direction get_split_dir_from_container_type();

  /* Determines whether to split horizontally or vertically based on container
   * type. */
  void split_containers();
  void parse_containers();

  /* Gets whether to split hori or vert based on which is longer. */
  gfwl_split_direction get_split_dir_longer();

  /* TODO: Finish
   * Close, unmap, and free this container. */
  void close();

  /* Descriptors of this container. */
  const gfwl_container_type e_type = GFWL_CONTAINER_UNKNOWN;
  const bool is_root = false;

  /* The Container Above and Below this one in the tiling tree. */
  std::weak_ptr<GfContainer> parent_container;
  std::vector<std::shared_ptr<GfContainer>> child_containers;

  /* The Dimensions of a container. */
  wlr_box box = {.x = 0, .y = 0, .width = 0, .height = 0};

  /* Tiling State of The Current Output. */
  std::weak_ptr<GfTilingState> tiling_state;

  /* References to the associated toplevel and server
   * TODO: I would later like to make this a shared pointer, container should
   * be the owner. */
  gfwl_toplevel *const toplevel;
  const gfwl_server &server;

private:
  /* These should only be called on split containers. These methods iterate
   * through all direct child containers and divide their height either
   * vertically or horizontally. */
  void vert_split_containers();
  void hori_split_containers();

  /* Sets the size and position of a container based on a wlr_box. */
  void set_container_box(struct wlr_box box);
};

void set_focused_toplevel_container(std::weak_ptr<GfContainer> container);
