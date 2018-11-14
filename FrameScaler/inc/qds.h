#ifndef QDS_H_
#define QDS_H_

#include "glm/glm.hpp"

void qds_insert_bounding_box(const glm::vec2& min_point, const glm::vec2& max_point);

void qds_update();

int get_recommended_framerate();

#endif // QDS_H_