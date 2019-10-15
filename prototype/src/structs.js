const InitCarAlignment = {
    Left: 1,
    Right: 2,
    Top: 3,
    Bottom: 4,
};

class Car {
    constructor() {
        this.position = createVector(0, 0);
        this.size = createVector(20, 40);
        this.direction = createVector(1, 0);
        this.speed = 2;
        this.wasInField = false;
        this.checkSides = true;
        this.setRandomColor();
    }

    setRandomColor() {
        colorMode(HSB);
        this.color = color(random(255), 255, 255, 255);
        colorMode(RGB);
    }

    getFutureRect() {
        return { x: this.position.x + this.direction.x * this.speed, y: this.position.y + this.direction.y * this.speed, w: this.size.x, h: this.size.y };
    }

    getForwardRect() {
        const forwardPos = p5.Vector.add(this.position, mult(this.direction, this.size));
        return { x: forwardPos.x, y: forwardPos.y, w: this.size.x, h: this.size.y };
    }

    getCenter() {
        return createVector(this.position.x + this.size.x / 2, this.position.y + this.size.y / 2);
    }

    getFrontPoint() {
        return { x: this.position.x + this.size.x / 2 + this.size.x * this.direction.x / 2, y: this.position.y + this.size.y / 2 + this.size.y * this.direction.y / 2 };
    }

    setAlignment(alignment) {
        switch (alignment) {
            case InitCarAlignment.Right:
                if (this.size.y > this.size.x) {
                    const tmp = this.size.x;
                    this.size.x = this.size.y;
                    this.size.y = tmp;
                }
                this.direction.x = -1;
                this.direction.y = 0;
                break;

            case InitCarAlignment.Top:
                if (this.size.y < this.size.x) {
                    const tmp = this.size.x;
                    this.size.x = this.size.y;
                    this.size.y = tmp;
                }
                this.direction.x = 0;
                this.direction.y = -1;
                break;

            case InitCarAlignment.Bottom:
                if (this.size.y < this.size.x) {
                    const tmp = this.size.x;
                    this.size.x = this.size.y;
                    this.size.y = tmp;
                }
                this.direction.x = 0;
                this.direction.y = 1;
                break;

            case InitCarAlignment.Left:
            default:
                if (this.size.y > this.size.x) {
                    const tmp = this.size.x;
                    this.size.x = this.size.y;
                    this.size.y = tmp;
                }
                this.direction.x = 1;
                this.direction.y = 0;
                break;
        }
    }
}

class Crossing {
    constructor(x, y, w, h) {
        this.position = createVector(x, y);
        this.size = createVector(w, h);
        this.cars = [];
    }
}