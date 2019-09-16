//
// Created by tvl on 10-09-19.
//

#include "flow_diagram.h"

#include "geoviz/flow_diagram/internal/test_internal.h"


namespace geoviz
{

std::string proc_flow_diagram()
{
  return "Flow Diagram (internal" + internal::test() + ")";
}

} // namespace geoviz