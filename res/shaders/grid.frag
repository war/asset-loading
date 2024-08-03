#version 330 core
out vec4 FragColor;

in vec3 WorldPos;

uniform vec3 cameraPos;

void main()
{
    vec3 lineColor = vec3(1.0, 1.0, 1.0);  // White lines
    
    float minorLineWidth = 0.5;
    float majorLineWidth = 2;
    
    vec2 coord = WorldPos.xz;
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);
    float minorLine = min(grid.x, grid.y);
    
    vec2 majorGrid = abs(fract(coord / 5.0 - 0.5) - 0.5) / fwidth(coord / 5.0);
    float majorLine = min(majorGrid.x, majorGrid.y);

    float line = min(minorLine, majorLine);
    
    // Determine the alpha based on whether we're on a line or not
    float alpha = 1.0 - smoothstep(0.0, minorLineWidth, minorLine);
    alpha = max(alpha, 1.0 - smoothstep(0.0, majorLineWidth, majorLine));
    
    // Apply distance fading
    float dist = length(WorldPos - cameraPos);
    alpha *= smoothstep(50.0, 30.0, dist);
    
    FragColor = vec4(lineColor, alpha);
}