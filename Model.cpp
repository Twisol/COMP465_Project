#include "Model.h"

Model loadTriangleModel() {
  Model model;

  // Make the model's GL state active
  glBindVertexArray(model.vao);
  glBindBuffer(GL_ARRAY_BUFFER, model.vbo);

  // Upload the model to GPU memory
  {
    GLfloat vertices[(3+4)*3] = {
        // position           color
        -10.0f, -10.0f, 0.0f,   1.0f, 0.0f, 0.0f, 1.0f,
        10.0f, -10.0f, 0.0f,   0.0f, 1.0f, 0.0f, 1.0f,
        0.0f,  10.0f, 0.0f,   0.0f, 0.0f, 1.0f, 1.0f,
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  }

  // Set attribute slot 0 to read the first 3 floats out of every set of 7 floats in the model.
  // In other words, slot 0 refers to the position data.
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7*sizeof(GLfloat), (GLvoid*)0);
  glEnableVertexAttribArray(0);  // TODO: What even is this? It's necessary, but what does it mean?

  // Slot 1 refers to the color data.
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7*sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
  glEnableVertexAttribArray(1);

  // As a precaution, unbind the vertex array so that GL operations don't accidentally operate on it.
  // The state and vertex information still exists - we can just bind model.vao to make it active again.
  glBindVertexArray(GL_NONE);

  return model;
}
