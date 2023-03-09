-- web server

function work(conn)
  if string.sub(path, 1, 5) == "/cmd/" then
    return http.string(conn, 200, path, "text/plain")
  end

  return false
end
