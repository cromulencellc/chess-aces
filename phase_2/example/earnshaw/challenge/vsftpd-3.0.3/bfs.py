from itertools import product


MEMO = {}


def bits(iterable):
    bit = 1
    res = 0
    for elem in iterable:
        if elem:
            res |= bit
        bit <<= 1
    return res


def mask(current, n):
    if (current, n) in MEMO:
        return MEMO[(current, n)]

    result = 0
    if current < n:
        for j in xrange(n):
            result += (2 ** ((current - 1)*n + j) + 2 ** (current*n + j))
    else:
        for i in xrange(n):
            result += (2 ** (i*n + current - n) + 2 ** (i*n + current - n + 1))

    MEMO[(current, n)] = result

    return result


# See: https://math.stackexchange.com/a/441697/4471
def check(matrix, n):
    parities = [sum(row) % 2 for row in matrix]
    for i in xrange(n):
        parities.append(sum([row[i] for row in matrix]) % 2)

    return len(set(parities)) == 1


def minimize(matrix, current, n):
    if current == 0:
        # See: https://stackoverflow.com/a/9831671/374865
        return bin(matrix).count("1")
    else:
        return min(minimize(matrix ^ mask(current, n), current - 1, n),
                   minimize(matrix, current - 1, n))


def solve(matrix, n):
    result = [0 for i in xrange(n) for j in xrange(n)]

    for i, j in product(xrange(n), repeat=2):
        if matrix[i][j]:
            for k in xrange(n):
                result[i*n + k] ^= 1
                result[k*n + j] ^= 1
            result[i*n + j] ^= 1

    if n % 2 == 0:
        return sum(result)
    else:
        return minimize(bits(result), 2*n - 2, n)


def answer(matrix):
    n = len(matrix)

    if n % 2 == 0:
        return solve(matrix, n)
    else:
        if check(matrix, n):
            return solve(matrix, n)
        else:
            return -1

'''
    a  b  c  d  e  f  g  h
 1  1  0  1  0  0  0  1  0
 2  1  1  0  1  0  0  1  0
 3  0  1  0  0  0  1  1  1
 4  1  0  1  0  0  0  0  0
 5  1  0  1  0  1  1  1  0
 6  1  1  0  1  0  0  0  1
 7  1  0  1  0  0  1  1  1
 8  0  1  1  1  1  1  1  0
 '''

 a = [ 
        [ 1, 0, 1, 0, 0, 0, 1, 0],
        [ 1, 1, 0, 1, 0, 0, 1, 0],
        [ 0, 1, 0, 0, 0, 1, 1, 1],
        [ 1, 0, 1, 0, 0, 0, 0, 0],
        [ 1, 0, 1, 0, 1, 1, 1, 0],
        [ 1, 1, 0, 1, 0, 0, 0, 1],
        [ 1, 0, 1, 0, 0, 1, 1, 1],
        [ 0, 1, 1, 1, 1, 1, 1, 0] ]

print(answer(a))