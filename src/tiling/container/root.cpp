#include "root.hpp"

#include <memory>
#include <output.hpp>

#include "tiling/container/base.hpp"
#include "tiling/state.hpp"
#include "wlr/util/box.h"

struct GfToplevel;

std::weak_ptr<GfContainer> GfContainerRoot::insert(GfToplevel* to_insert) {
  return this->insert_child_in_split(to_insert, GFWL_CONTAINER_HSPLIT);
}

void GfContainerRoot::set_to_output_size() {
  std::shared_ptr<GfOutput> output = this->tiling_state.lock()->output;
  this->box                        = output->get_usable_space();
}

/* If we are in the root we need to set the root container to the size of the
 * output. */
void GfContainerRoot::parse_containers() {
  this->set_to_output_size();
  this->split_containers();
  this->parse_children();
}
