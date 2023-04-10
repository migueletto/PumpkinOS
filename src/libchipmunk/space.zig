const ch = @cImport({
  @cInclude("chipmunk.h");
});

pub const SpaceBody = struct {
  space: ?*ch.cpSpace = null,
  body: ?*ch.cpBody = null,
  shape: ?*ch.cpShape = null,
  rx: f64 = 0,
  ry: f64 = 0,
  w: i16 = 0,
  h: i16 = 0,

  pub fn setPosition(self: SpaceBody, x: f64, y: f64) void {
    ch.cpBodySetPosition(self.body, ch.cpv(x, y));
  }

  pub fn getPosition(self: SpaceBody, x: *f64, y: *f64) void {
    var pos: ch.cpVect = ch.cpBodyGetPosition(self.body);
    x.* = pos.x;
    y.* = pos.y;
  }

  pub fn setVelocity(self: SpaceBody, x: f64, y: f64) void {
    ch.cpBodySetVelocity(self.body, ch.cpv(x, y));
  }

  pub fn setAngularVelocity(self: SpaceBody, r: f64) void {
    ch.cpBodySetAngularVelocity(self.body, r);
  }

  pub fn getAngle(self: SpaceBody) f64 {
    return ch.cpBodyGetAngle(self.body);
  }

  pub fn setFriction(self: SpaceBody, f: f64) void {
    ch.cpShapeSetFriction(self.shape, f);
  }

  pub fn setElasticity(self: SpaceBody, e: f64) void {
    ch.cpShapeSetElasticity(self.shape, e);
  }
};

pub const Space2D = struct {
  space: ?*ch.cpSpace = null,
  time: ch.cpFloat = 0,

  pub fn setGravity(self: Space2D, x: f64, y: f64) void {
    var gravity = ch.cpv(x, y);
    ch.cpSpaceSetGravity(self.space, gravity);
  }

  pub fn addBox(self: *Space2D, width: f64, height: f64, mass: f64) SpaceBody {
    var body: ?*ch.cpBody = null;
    if (mass >= 0) {
      var moment = ch.cpMomentForBox(mass, width, height);
      body = ch.cpBodyNew(mass, moment);
      _ = ch.cpSpaceAddBody(self.space, body);
    } else {
      body = ch.cpSpaceGetStaticBody(self.space);
    }
    var shape = ch.cpBoxShapeNew(body, width, height, 0);
    _ = ch.cpSpaceAddShape(self.space, shape);

    var box = SpaceBody {
      .space = self.space,
      .body = body,
      .shape = shape,
      .rx = width / 2,
      .ry = height / 2,
    };

    return box;
  }

  pub fn addCircle(self: *Space2D, radius: f64, mass: f64) SpaceBody {
    var body: ?*ch.cpBody = null;
    if (mass >= 0) {
      var moment = ch.cpMomentForCircle(mass, 0, radius, ch.cpvzero);
      body = ch.cpBodyNew(mass, moment);
      _ = ch.cpSpaceAddBody(self.space, body);
    } else {
      body = ch.cpSpaceGetStaticBody(self.space);
    }
    var shape = ch.cpCircleShapeNew(body, radius, ch.cpvzero);
    _ = ch.cpSpaceAddShape(self.space, shape);

    var width = @floatToInt(i16, radius * 2);
    var height = @floatToInt(i16, radius * 2);

    var circle = SpaceBody {
      .space = self.space,
      .body = body,
      .shape = shape,
      .rx = radius,
      .ry = radius,
      .w = width,
      .h = height,
    };

    return circle;
  }

  pub fn addSegment(self: *Space2D, x1: f64, y1: f64, x2: f64, y2: f64, mass: f64) SpaceBody {
    var v1 = ch.cpv(x1, y1);
    var v2 = ch.cpv(x2, y2);
    var body: ?*ch.cpBody = null;
    if (mass >= 0) {
      var moment = ch.cpMomentForSegment(mass, v1, v2, 0);
      body = ch.cpBodyNew(mass, moment);
      _ = ch.cpSpaceAddBody(self.space, body);
    } else {
      body = ch.cpSpaceGetStaticBody(self.space);
    }
    var shape = ch.cpSegmentShapeNew(body, v1, v2, 0);
    _ = ch.cpSpaceAddShape(self.space, shape);

    return SpaceBody {
      .space = self.space,
      .body = body,
      .shape = shape,
    };
  }

  pub fn step(self: Space2D, timeStep: f64) void {
    ch.cpSpaceStep(self.space, timeStep);
  }

  pub fn deinit(self: *Space2D) void {
    ch.cpSpaceEachShape(self.space, shapeCallback, null);
    ch.cpSpaceEachBody(self.space, bodyCallback, null);
    ch.cpSpaceFree(self.space);
    self.space = null;
  }
};

fn shapeCallback(shape: ?*ch.cpShape, data: ?*anyopaque) callconv(.C) void {
  _ = data;
  ch.cpShapeFree(shape);
}

fn bodyCallback(body: ?*ch.cpBody, data: ?*anyopaque) callconv(.C) void {
  _ = data;
  ch.cpBodyFree(body);
}

pub fn init() Space2D {
  var space = Space2D {
    .space = ch.cpSpaceNew(),
  };

  return space;
}
