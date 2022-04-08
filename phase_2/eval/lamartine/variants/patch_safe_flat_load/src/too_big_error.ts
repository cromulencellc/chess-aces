export class TooBigError extends Error {
    constructor() {
        super(`this map is too big for javascript integers`);
    }
}
