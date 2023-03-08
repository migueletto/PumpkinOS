-- Game of Life

-- Life class
Life = {}

fg = screen.rgb(0xff, 0xff, 0xff) -- foreground color
bg = screen.rgb(0x30, 0x30, 0xff) -- background color
wcell = 8 -- cell width in pixels
hcell = 8 -- cell height in pixels
y0 = 12   -- grid top y position (after title)

-- create a new instance of the Life class
function Life.new(n, m)
  local life = {} -- Life object
  life.m = m
  life.n = n

  -- clears the grid
  life.clear = function (self)
    self.matrix = {}
    self.age = {}

    for i = 1, m do
      local row = {}
      for j = 1, n do
        row[j] = 0
      end
      self.matrix[i] = row
    end

    for i = 1, m do
      local row = {}
      for j = 1, n do
        row[j] = 0
      end
      self.age[i] = row
    end
  end

  --eset a grid cell
  life.set = function (self, x, y)
    self.matrix[y][x] = 1
  end

  -- reset a grid cell
  life.reset = function (self, x, y)
    self.matrix[y][x] = 0
  end

  -- invert a grid cell
  life.invert = function (self, x, y)
    if self.matrix[y][x] > 0 then
      self.matrix[y][x] = 0
    else
      self.matrix[y][x] = 1
    end
  end

  -- advance the simulation state
  life.advance = function (self)
    local aux = deepcopy(self.matrix)
    for i = 1, self.m do
      for j = 1, self.n do
        local s = 0
        for p = i-1,i+1 do
          for q = j-1,j+1 do
            if p > 0 and p <= self.m and q > 0 and q <= self.n then
              s = s + self.matrix[p][q]
            end
          end
        end

        s = s - self.matrix[i][j]

        if s == 3 or (s+self.matrix[i][j]) == 3 then
          if aux[i][j] == 1 then
            self.age[i][j] = self.age[i][j] + 1
          else
            aux[i][j] = 1
            self.age[i][j] = 0
          end
        else
          aux[i][j] = 0
          self.age[i][j] = 0
        end
      end
    end
    self.matrix = aux
  end

  -- draw the whole grid
  life.draw = function (self)
    for i = 1, self.m do
      for j = 1, self.n do
        local c
        if self.matrix[i][j] == 0 then
          -- cell is empty, use backgound color
          c = bg
        else 
          -- cells get a red color as they age
          local age = self.age[i][j]
          if age > 63 then age = 63 end
          c = screen.rgb(255, 255-age*4, 255-age*4)
        end
        -- draw a single cell (filled rectangle of color c)
        screen.rect((j-1)*wcell, y0+(i-1)*hcell, wcell, hcell, true, c)
      end
    end  
  end

  -- new instance starts empty
  life:clear()

  -- return the new instance
  return life
end

-- performs a deep copy of a table
function deepcopy(object)
  local lookup_table = {}
  local function _copy(object)
    if type(object) ~= "table" then
      return object
    elseif lookup_table[object] then
      return lookup_table[object]
    end
    local new_table = {}
    lookup_table[object] = new_table
    for index, value in pairs(object) do
      new_table[_copy(index)] = _copy(value)
    end
    return new_table
  end
  return _copy(object)
end

function title()
  local status = "P" -- paused
  if running then status = "R" end -- running
  -- prints title and status on top of screen
  screen.draw("Game of Life (" .. status .. ")", 50, 0, fg, bg)
end

-- calculate grid size
xcells = math.floor(screen.width() / wcell)
ycells = math.floor((screen.height() - y0) / hcell)

-- create a Life object
life = Life.new(xcells, ycells)

screen.clear(bg) -- clear screen with background color
running = false  -- start in the paused state
title()          -- print title

-- loop forever
while true do
  -- wait 10 ticks for events
  ev = ui.event(10)

  if ev.type == nilEvent then
    -- nilEvent is sent when no other user event occurred
    if running then
      -- if running (not paused), animate simulation
      life:advance()
      life:draw()
    end
  elseif ev.type == penDown then
    -- penDown event received
    if not running then
      -- invert selected cell
      local x = math.floor(ev.x / wcell) + 1
      local y = math.floor((ev.y - y0) / hcell) + 1
      if x >= 1 and x <= xcells and y >= 1 and y <= ycells then
        life:invert(x,y)
        life:draw()
      end
    end
  elseif ev.type == keyDown then
    -- keyDown event received
    if string.char(ev.key) == "r" then
      -- 'r' key pressed, toggle running/paused state
      running = not running
      -- redraw title to reflect new state
      title()
    end
  elseif ev.type == appStop then
    -- appStop event received, quit
    break
  end
end
