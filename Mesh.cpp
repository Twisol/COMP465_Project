#include "Mesh.h"

Mesh loadTriangleMesh() {
  Mesh mesh;

  // Make the model's GL state active
  glBindVertexArray(mesh.vao);
  glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

  // Upload the model to GPU memory
  {
    GLfloat vertices[(3+4)*3] = {
        // position           color
        -1.0f, -1.0f, 0.0f,   1.0f, 0.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f, 1.0f,
         0.0f,  1.0f, 0.0f,   0.0f, 0.0f, 1.0f, 1.0f,
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    mesh.primitiveType = GL_TRIANGLES;
    mesh.primitiveCount = 3;
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

  return mesh;
}

Mesh loadSphereMesh() {
  Mesh mesh;

  // Make the model's GL state active
  glBindVertexArray(mesh.vao);
  glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

  // Upload the model to GPU memory
  {
    GLfloat vertices[(3+4)*6*6] = {
        // Front face
        -1.0f,  1.0f,  1.0f,   1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,   1.0f, 0.0f, 0.0f, 1.0f,
         1.0f, -1.0f,  1.0f,   1.0f, 0.0f, 0.0f, 1.0f,
         1.0f, -1.0f,  1.0f,   0.5f, 0.0f, 0.0f, 1.0f,
         1.0f,  1.0f,  1.0f,   0.5f, 0.0f, 0.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,   0.5f, 0.0f, 0.0f, 1.0f,

        // Back face
        -1.0f,  1.0f, -1.0f,   1.0f, 1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, -1.0f,   1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,   1.0f, 1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, -1.0f,   0.5f, 0.5f, 0.0f, 1.0f,
        -1.0f,  1.0f, -1.0f,   0.5f, 0.5f, 0.0f, 1.0f,
         1.0f,  1.0f, -1.0f,   0.5f, 0.5f, 0.0f, 1.0f,

        // Left face
        -1.0f,  1.0f, -1.0f,   0.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,   0.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,   0.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,   0.0f, 0.5f, 0.5f, 1.0f,
        -1.0f,  1.0f,  1.0f,   0.0f, 0.5f, 0.5f, 1.0f,
        -1.0f,  1.0f, -1.0f,   0.0f, 0.5f, 0.5f, 1.0f,

        // Right face
         1.0f,  1.0f, -1.0f,   0.0f, 1.0f, 0.0f, 1.0f,
         1.0f, -1.0f,  1.0f,   0.0f, 1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, -1.0f,   0.0f, 1.0f, 0.0f, 1.0f,
         1.0f, -1.0f,  1.0f,   0.0f, 0.5f, 0.0f, 1.0f,
         1.0f,  1.0f, -1.0f,   0.0f, 0.5f, 0.0f, 1.0f,
         1.0f,  1.0f,  1.0f,   0.0f, 0.5f, 0.0f, 1.0f,

        // Top face
        -1.0f,  1.0f, -1.0f,   1.0f, 0.0f, 1.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,   1.0f, 0.0f, 1.0f, 1.0f,
         1.0f,  1.0f,  1.0f,   1.0f, 0.0f, 1.0f, 1.0f,
         1.0f,  1.0f,  1.0f,   0.5f, 0.0f, 0.5f, 1.0f,
         1.0f,  1.0f, -1.0f,   0.5f, 0.0f, 0.5f, 1.0f,
        -1.0f,  1.0f, -1.0f,   0.5f, 0.0f, 0.5f, 1.0f,

        // Bottom face
        -1.0f, -1.0f, -1.0f,   0.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f,  1.0f,   0.0f, 0.0f, 1.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,   0.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f,  1.0f,   0.0f, 0.0f, 0.5f, 1.0f,
        -1.0f, -1.0f, -1.0f,   0.0f, 0.0f, 0.5f, 1.0f,
         1.0f, -1.0f, -1.0f,   0.0f, 0.0f, 0.5f, 1.0f,
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    mesh.primitiveType = GL_TRIANGLES;
    mesh.primitiveCount = 6*6;
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

  return mesh;
}
