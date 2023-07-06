

function toBitBoard(locations) {
    const i = new Array(8).fill(0)
    for (let location of locations) {
        const { x, y } = location;

        i[7 - y] |= ((1 >>> 0) << x);
    }
    return i.map(i => (i >>> 0).toString(16).padStart(2, "0")).join("")
}

function loc(x, y) {
    return { x, y };
}

function isValid(loc) {
    const { x, y } = loc;
    if (x < 0 || x >= 8) return false
    if (y < 0 || y >= 8) return false
    return true
}

const kingMoves = []
const knightMoves = []
const pinRays = [[], [], [], []]
const directions = [
    [loc(-1, +1), loc(+1, -1)],
    [loc(+0, +1), loc(+0, -1)],
    [loc(+1, +1), loc(-1, -1)],
    [loc(+1, +0), loc(-1, +0)],
]

for (let y = 0; y < 8; y++) {
    for (let x = 0; x < 8; x++) {
        kingMoves.push([
            loc(x + 1, y + 1), loc(x + 0, y + 1), loc(x - 1, y + 1),
            loc(x + 1, y + 0), loc(x - 1, y + 0),
            loc(x + 1, y - 1), loc(x + 0, y - 1), loc(x - 1, y - 1),
        ])
        knightMoves.push([
            loc(x - 1, y + 2), loc(x + 1, y + 2),
            loc(x - 2, y + 1), loc(x + 2, y + 1),
            loc(x - 2, y - 1), loc(x + 2, y - 1),
            loc(x - 1, y - 2), loc(x + 1, y - 2),
        ])
        for (let i = 0; i < directions.length; i++) {
            let pinRay = [];
            for (let dir of directions[i]) {
                for (let i = 0; i < 8; i++) {
                    pinRay.push(loc(x + i * dir.x, y + i * dir.y))
                }
            }
            pinRays[i].push(pinRay)
        }
    }
}
//const movesStr = pinRays
//    .map(x => toBitBoard(x.filter(isValid)))
//    .map(x => "0x" + x + "ULL, ")
const movesStr = new Array(64).fill()
    .map((x, i) => [pinRays[0][i], pinRays[1][i], pinRays[2][i], pinRays[3][i]])
    .map(x => x.map(x => toBitBoard(x.filter(isValid))).map(x => "0x" + x + "ULL"))
    .map(x => "{" + x.join(", ") + "}, ")
let line = ""
for (let i = 0; i < 64; i++) {
    if ((i % 8) == 0) {
        console.log(line)
        line = ""
    }
    line += movesStr[i]
}
console.log(line)
