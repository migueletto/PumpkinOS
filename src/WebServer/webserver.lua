-- web server

function dumpb(s)
  local b = string.sub(s, 1, 128)
  pit.debugbytes(1, b)
end

function dumpe(s)
  local b = string.sub(s, -128)
  pit.debugbytes(1, b)
end

function worker(conn)
  if string.sub(path, 1, 7) == "/upload" then
    pit.debug(1, "method = " .. method)
    pit.debug(1, "path = " .. path)

    for key,value in pairs(header) do
      pit.debug(1, "header [" .. key .. "] = [" .. value .. "]")
    end
    for key,value in pairs(param) do
      pit.debug(1, "param [" .. key .. "] = [" .. value .. "]")
    end

    -- Content-Type: multipart/form-data; boundary=---------------------------24866831032539771641151234281
    local contentType = header["Content-Type"]
    if contentType then
      local prefix = "multipart/form-data; boundary="
      local len = string.len(prefix)
      local aux = string.sub(contentType, 1, len)
      if aux == prefix then
        local boundary = string.sub(contentType, len + 1)
        pit.debug(1, "boundary [" .. boundary .. "]")
        local body = ""
        while true do
          local s = http.read(conn, 1024)
          if not s then break end
          if s == "" then break end
          body = body .. s
        end
        local bodylen = string.len(body)
        pit.debug(1, "body has " .. bodylen .. " bytes")
        dumpb(body)
        len = string.len(boundary)
        local i = string.find(body, boundary, 1, true)
        if i then
          body = string.sub(body, i + len + 2)
          pit.debug(1, "body after boundary")
          dumpb(body)
          i = string.find(body, "\r\n\r\n", 1, true)
          if i then
            body = string.sub(body, i + 4)
            pit.debug(1, "body after cr lf cr lf")
            dumpb(body)
            i = string.find(body, boundary, 1, true)
            if i then
              body = string.sub(body, 1, i - 1)
              if string.sub(body, -4) == "\r\n--" then
                body = string.sub(body, 1, -5)
              end
              pit.debug(1, "payload end")
              dumpe(body)
              bodylen = string.len(body)
              pit.debug(1, "payload has " .. bodylen .. " bytes")
              http.install(body)
            else
              pit.debug(1, "body does not end with boundary")
            end
          else
            pit.debug(1, "body does not have cr lf cr lf")
          end
        else
          pit.debug(1, "body does not start with boundary")
        end
      else
        pit.debug(1, "invalid Content-Type")
      end
    else
     pit.debug(1, "no Content-Type header")
    end

    return http.string(conn, 200, "OK", "text/plain")
  end

  return false
end
