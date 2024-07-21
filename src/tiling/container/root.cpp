#include "root.hpp"
#include <output.hpp>

std::weak_ptr<GfContainer> GfContainerRoot::insert(gfwl_toplevel* to_insert) {
  return this->insert_child_in_split(to_insert, GFWL_CONTAINER_HSPLIT);
}

void GfContainerRoot::set_to_output_size() {
  std::shared_ptr<gfwl_output> output = this->tiling_state.lock()->output;
  this->box.x                         = output->scene_output->x;
  this->box.y                         = output->scene_output->y;
  this->box.width                     = output->wlr_output->width;
  this->box.height                    = output->wlr_output->height;
}

/* If we are in the root we need to set the root container to the size of the
 * output. */
void GfContainerRoot::parse_containers() {
  this->set_to_output_size();
  this->split_containers();
  this->parse_children();
}
