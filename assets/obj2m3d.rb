# Export as Z forward, -Y up


vertices = []
normals = []
uvs = []
faces = []

# TODO : Command line args
texture_file = "none"
include_normal = false
texture_w = 96
texture_h = 64
BASE_SIZE = 75


File.readlines('quad.obj').each do |line|
  if line.start_with?('v ')
    # puts 'vertice'
    vertices << line.chomp.split(' ')[1..].map { |v| (v.to_f * BASE_SIZE).round }
  elsif line.start_with?('vn ')
    # puts 'normal'
    normals << line.chomp.split(' ')[1..].map(&:to_f)
  elsif line.start_with?('vt ')
    # puts 'uv'
    uv = line.chomp.split(' ')[1..].map(&:to_f)
    # uvs << [uv[0] * texture_w, uv[1] * texture_h].map(&:round)
    uvs << [uv[0] * texture_w, (1 - uv[1]) * texture_h].map(&:round)
  elsif line.start_with?('f ')
    # f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 [v4/vt4/vn4]
    # puts 'face'
    raw_faces = line.chomp.split(' ')[1..]
    o_faces = []
    raw_faces.each do |face|
      o_faces << face.split('/').map { |f| f.to_i - 1 }
    end
    # p raw_faces
    # p o_faces
    faces << o_faces
  end
end

output_faces = []
output_vertices = []

faces.each do |face|
  output_face = []
  output_face << face.count

  face.each do |v|
    v_i = v[0]
    vt_i = v[1]
    vn_i = v[2]
    vertex = vertices[v_i]
    normal = normals[vn_i]
    uv = uvs[vt_i]

    if include_normal
      output_vertex = (vertex + normal + uv)
    else
      output_vertex = (vertex + uv)
    end

    if !output_vertices.include?(output_vertex)
      output_vertices << output_vertex
    end

    output_face << output_vertices.index(output_vertex)
  end

  output_faces << output_face
end

# output file

puts "vertices_count: #{output_vertices.size}"
puts "faces_count: #{output_faces.size}"
puts "texture_file: #{texture_file}"

output_vertices.each do |vertice|
  puts vertice.join(' ')
end
output_faces.each do |face|
  puts face.join(' ')
end


# puts "vertices_count: #{vertices.size}"
# puts "faces_count: #{faces.size}"
# puts "texture_file: #{texture_file}"

# vertices.each do |vertice|
#   puts (vertice + [0, 0]).join(' ')
# end

# faces.each do |face|
#   vi = [face.size]
#   face.each do |v|
#     vi << (v[0] + v[1])
#   end
#   puts vi.join(' ')
# end
