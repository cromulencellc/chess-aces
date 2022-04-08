export class DefaultMap<KeyType, ValueType> {
    default_maker: () => ValueType
    container: Map<KeyType, ValueType>

    constructor(default_maker: () => ValueType) {
        this.container = new Map()
        this.default_maker = default_maker
    }

    get(key: KeyType): ValueType {
        let got = this.container.get(key)
        if (undefined != got) return got

        let made = this.default_maker()
        this.container.set(key, made)
        return made
    }

    set(key: KeyType, value: ValueType) {
        this.container.set(key, value)
    }

    [Symbol.iterator](): Iterator<[KeyType, ValueType]> {
        return this.container[Symbol.iterator]()
    }
}