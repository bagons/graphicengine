# Shader gen

## what is it? why?

All ShaderPrograms require a **vertex** and a **fragment** shader. Vertex shader handles the correct projection to a 2D screen. And a fragment shader handles the visual aspect. But these shaders can be tedious to write and there is a lot of overlap.

That's where ShaderGen comes in. It generates appropriate shader code using pre-coded templates.

But, the **game engine** still wants you give you a way to write you you're own shaders and leverage the ShaderGens power.

So that's why this manual exists.