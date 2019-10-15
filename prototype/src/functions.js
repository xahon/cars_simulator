
function intersects(rect1, rect2, useEdgeCornerIntersection = true) {
    const p1 = createVector(rect1.x, rect1.y);
    const p2 = createVector(rect1.x + rect1.w, rect1.y + rect1.h);

    const op1 = createVector(rect2.x, rect2.y);
    const op2 = createVector(rect2.x + rect2.w, rect2.y + rect2.h);

    if (useEdgeCornerIntersection) {
        return p1.x <= op2.x &&
            p2.x >= op1.x &&
            p1.y <= op2.y &&
            p2.y >= op1.y;
    }

    return p1.x < op2.x &&
        p2.x > op1.x &&
        p1.y < op2.y &&
        p2.y > op1.y;
}

function getRect() {
    if (arguments.length == 1) {
        if (arguments[0] instanceof Car) {
            return { x: arguments[0].position.x, y: arguments[0].position.y, w: arguments[0].size.x, h: arguments[0].size.y };
        } else if (arguments[0] instanceof Crossing) {
            return { x: arguments[0].position.x, y: arguments[0].position.y, w: arguments[0].size.x, h: arguments[0].size.y };
        }
    } else if (arguments.length == 2) {
        if (arguments[0] instanceof p5.Vector && arguments[1] instanceof p5.Vector) {
            return { x: arguments[0].x, y: arguments[0].y, w: arguments[1].x, h: arguments[1].y };
        }
        return { x: 0, y: 0, w: arguments[0], h: arguments[1] };
    } else if (arguments.length == 4) {
        return { x: arguments[0], y: arguments[1], w: arguments[2], h: arguments[3] };
    }
}

function rightPerpendicular(vector) {
    if (vector instanceof p5.Vector) {
        return createVector(vector.y, -vector.x);
    } else if (arguments.length == 2) {
        return createVector(arguments[1], -arguments[0]);
    }
}

function leftPerpendicular(vector) {
    if (vector instanceof p5.Vector) {
        return createVector(-vector.y, vector.x);
    } else if (arguments.length == 2) {
        return createVector(-arguments[1], arguments[0]);
    }
}

function copyObject(object) {
    if (object instanceof p5.Color) {
        if (object.mode == RGB)
            return color(red(object), green(object), blue(object), alpha(object));
        if (object.mode == HSL)
            return color(hue(object), saturation(object), lightness(object), alpha(object));
        if (object.mode == HSB)
            return color(hue(object), saturation(object), brightness(object), alpha(object));
    }
}

function mult(object) {
    if (arguments.length == 2) {
        if (arguments[0] instanceof p5.Vector && arguments[1] instanceof p5.Vector) {
            return createVector(arguments[0].x * arguments[1].x, arguments[0].y * arguments[1].y, arguments[0].z * arguments[1].z);
        } else if (arguments[1] instanceof p5.Vector && typeof arguments[0] === "number" || arguments[0] instanceof p5.Vector && typeof arguments[1] === "number") {
            const numberIdx = typeof arguments[0] == "number" ? 0 : 1;
            const number = arguments[numberIdx];
            const vector = arguments[numberIdx === 1 ? 0 : 1];
            return p5.Vector.mult(vector, number);
        }
    }
}

function sign(number) {
    if (number < 0) return -1;
    if (number > 0) return -1;
    return 0;
}