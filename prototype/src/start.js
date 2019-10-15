let fontStyle;
const scrWidth = 640;
const scrHeight = 640;
const carsCount = 10;

const crossings = [];
const cars = [];
const laneSize = 40;
const road = {
    leftBottomHorizontal: {},
    rightTopHorizontal: {},
    leftTopVertical: {},
    rightBottomVertical: {},
    crossing: {},
    leftSpawn: {},
    rightSpawn: {},
    topSpawn: {},
    bottomSpawn: {},
};

function preload() {
    fontStyle = loadFont('https://cdnjs.cloudflare.com/ajax/libs/topcoat/0.8.0/font/SourceCodePro-Regular.otf');
}

function setup() {
    road.leftBottomHorizontal = { x: 0, y: scrHeight / 2 - laneSize };
    road.rightTopHorizontal = { x: scrWidth, y: scrHeight / 2 + laneSize };
    road.leftTopVertical = { x: scrWidth / 2 - laneSize, y: scrHeight };
    road.rightBottomVertical = { x: scrWidth / 2 + laneSize, y: 0 };
    road.crossing = new Crossing(road.leftTopVertical.x, road.leftBottomHorizontal.y, laneSize * 2, laneSize * 2);
    road.leftSpawn = { x: road.leftBottomHorizontal.x, y: road.leftBottomHorizontal.y };
    road.rightSpawn = { x: road.rightTopHorizontal.x, y: road.rightTopHorizontal.y - laneSize };
    road.topSpawn = { x: road.leftTopVertical.x, y: road.leftTopVertical.y };
    road.bottomSpawn = { x: road.rightBottomVertical.x - laneSize, y: road.rightBottomVertical.y };

    createCanvas(scrWidth, scrHeight);
    initCars();
    resolveCollisions();

    rectMode(CORNER);
    colorMode(RGB, 255);
}

function draw() {
    drawBackground();

    resolveCollisions();
    respawnOutOfFieldCars();
    const moveData = resolveMovings();
    resolveDeadlocks();
    handleMovings(moveData);
    drawCars();
}

function createRandomCar() {
    const car = new Car();
    const randomPosition = random([road.leftSpawn, road.rightSpawn, road.topSpawn, road.bottomSpawn]);
    const isLTPositioned = randomPosition == road.leftSpawn || randomPosition == road.topSpawn;
    const isLBPositioned = randomPosition == road.leftSpawn || randomPosition == road.bottomSpawn;
    car.position = createVector(randomPosition.x, randomPosition.y);

    car.setAlignment(
        randomPosition == road.leftSpawn
            ? InitCarAlignment.Left
            : randomPosition == road.rightSpawn
                ? InitCarAlignment.Right
                : randomPosition == road.topSpawn
                    ? InitCarAlignment.Top
                    : InitCarAlignment.Bottom
    );

    if (isLTPositioned) {
        car.position.add(mult(leftPerpendicular(car.direction), mult(car.size, 0.5)));
    } else {
        car.position.add(mult(rightPerpendicular(car.direction), mult(car.size, 0.5)));
    }
    if (isLBPositioned) {
        car.position.sub(mult(car.size, car.direction));
    }

    return car;
}

function initCars() {
    for (let i = 0; i < carsCount; ++i) {
        const car = createRandomCar();
        cars.push(car);
    }
}

function respawnOutOfFieldCars() {
    const screenRect = getRect(scrWidth, scrHeight);
    let respawned = false;
    _.each(cars, (car, i) => {
        const carRect = getRect(car);
        if (!car.wasInField) {
            if (intersects(carRect, screenRect)) {
                car.wasInField = true;
            }
        } else {
            if (!intersects(carRect, screenRect)) {
                cars[i] = createRandomCar();
                respawned = true;
            }
        }
    });

    if (respawned)
        resolveCollisions();
}

function resolveCollisions() {
    const queue = [...cars];
    while (queue.length > 0) {
        const car = queue.pop();

        _.each(queue, otherCar => {
            if (car == otherCar)
                return;
            if (intersects(getRect(car), getRect(otherCar), false)) {
                do {
                    car.position.sub(p5.Vector.mult(car.direction, car.speed));
                } while (intersects(getRect(car), getRect(otherCar), false));
            }
        })
    }
}

function resolveMovings() {
    const moveData = [];
    _.each(cars, (car, i) => {
        const crossingIntersection = intersects(getRect(road.crossing), getRect(car));
        road.crossing.cars = road.crossing.cars.filter(c => c != car);
        if (crossingIntersection) {
            road.crossing.cars.push(car);
        }
    });

    _.each(cars, (car, i) => {
        let shouldMove = true;
        const fwdRect = car.getForwardRect();

        _.each(cars, (otherCar, j) => {
            if (i == j)
                return;
            if (intersects(fwdRect, otherCar.getFutureRect(), false)) {
                shouldMove = false;
            }
        });

        const crossingIntersection = intersects(getRect(road.crossing), getRect(car));

        if (!crossingIntersection && !car.checkSides) {
            car.checkSides = true;
        }

        if (!car.checkSides) {
            shouldMove = true;
        }
        if (car.checkSides && crossingIntersection) {
            const carFromRight = road.crossing.cars.find(c => c.direction.equals(leftPerpendicular(car.direction)));

            if (carFromRight != null) {
                if (!intersects(fwdRect, getRect(road.crossing))) { // is going out of crossing
                    shouldMove = true;
                } else {
                    shouldMove = false;
                }
            } else {
                shouldMove = true;
            }
        }

        let newPos;
        if (shouldMove) {
            newPos = p5.Vector.add(car.position, p5.Vector.mult(car.direction, car.speed));
        } else {
            newPos = car.position;
        }

        moveData[i] = {
            car,
            newPos,
        };
    });

    return moveData;
}

function resolveDeadlocks() {
    if (road.crossing.cars.length <= 3)
        return;
    let hasRight, hasLeft, hasTop, hasBottom;
    for (const car of road.crossing.cars) {
        if (hasRight && hasLeft && hasTop && hasBottom) {
            break;
        }
        if (car.direction.x == 1) {
            hasLeft = true;
        }
        if (car.direction.x == -1) {
            hasRight = true;
        }
        if (car.direction.y == 1) {
            hasBottom = true;
        }
        if (car.direction.y == -1) {
            hasTop = true;
        }
    }
    if (!hasRight || !hasLeft || !hasTop || !hasBottom) {
        return;
    }

    const sortedCars = road.crossing.cars.sort((a, b) => a.position.x < b.position.x ? -1 : 1);

    let carToCrossFirst;
    for (const car of sortedCars) {
        const bigForwardCast = car.getForwardRect();
        if (car.direction.y == 0)
            bigForwardCast.w = road.crossing.size.x;
        if (car.direction.x == 0)
            bigForwardCast.h = road.crossing.size.y;
        let carFound = true;
        for (const crossingCar of road.crossing.cars) {
            if (car == crossingCar)
                continue;
            if (intersects(bigForwardCast, getRect(crossingCar), false)) {
                carToCrossFirst = car;
                carFound = false;
                break;
            }
        }
        if (carFound) {
            carToCrossFirst = car;
            break;
        }
    }
    carToCrossFirst.checkSides = false;
}

function handleMovings(moveData) {
    _.each(moveData, md => {
        md.car.position = md.newPos;
    });
}

function drawBackground() {
    background(0, 160, 0);

    // Left-to-Right road
    fill(51);
    noStroke();
    rect(road.leftBottomHorizontal.x, road.leftBottomHorizontal.y, scrWidth, laneSize);
    rect(road.leftBottomHorizontal.x, road.leftBottomHorizontal.y + laneSize, scrWidth, laneSize);

    // Top-to-Bottom road
    fill(51);
    noStroke();
    rect(road.leftTopVertical.x, road.rightBottomVertical.y, laneSize, scrHeight);
    rect(road.leftTopVertical.x + laneSize, road.rightBottomVertical.y, laneSize, scrHeight);

    // Marking
    drawingContext.lineDashOffset = 0;
    drawingContext.setLineDash([10, 15]);
    stroke(255, 255, 255, 255);
    strokeWeight(2);
    line(road.leftBottomHorizontal.x, road.leftBottomHorizontal.y + laneSize, road.leftBottomHorizontal.x + scrWidth, road.leftBottomHorizontal.y + laneSize);
    line(road.leftTopVertical.x + laneSize, road.leftTopVertical.y, road.leftTopVertical.x + laneSize, 0);

    // Crossing
    drawingContext.lineDashOffset = laneSize;
    drawingContext.setLineDash([laneSize]);
    fill(49);
    stroke(255, 255, 255, 255);
    strokeWeight(2);
    rect(..._.values(getRect(road.crossing)));

    // Spawn points
    stroke(255, 0, 0, 255);
    strokeWeight(4);
    point(road.leftSpawn.x, road.leftSpawn.y);
    point(road.rightSpawn.x, road.rightSpawn.y);
    point(road.topSpawn.x, road.topSpawn.y);
    point(road.bottomSpawn.x, road.bottomSpawn.y);
}

function drawCars() {
    _.each(cars, (car, i) => {
        noStroke();
        fill(car.color);
        rect(..._.values(getRect(car)));

        const center = car.getCenter();
        const lineEnd = p5.Vector.add(center, mult(car.direction, car.size));
        drawingContext.lineDashOffset = 0;
        drawingContext.setLineDash([0]);
        strokeWeight(2);
        stroke(0, 0, 0, 255);
        line(center.x, center.y, lineEnd.x, lineEnd.y);
        strokeWeight(4);
        point(car.getFrontPoint().x, car.getFrontPoint().y);

        push()
        noStroke()
        noFill()
        textFont(fontStyle);
        textAlign(CENTER, CENTER);
        textSize(24);
        strokeWeight(1);
        stroke(0, 0, 0, 255);
        fill(0, 0, 0, 255);
        translate(car.getCenter().x, car.getCenter().y);
        scale(1, -1);
        text(i.toString(), 0, 0);
        pop()
    });
}