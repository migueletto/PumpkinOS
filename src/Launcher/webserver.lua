-- web server

t = template.create("/templates/test.html", "text/html")
x = 0

function work(conn)
  if string.sub(path, 1, 5) == "/cmd/" then
    x = x + 1
    return http.template(conn, t)
  end

  return false
end
