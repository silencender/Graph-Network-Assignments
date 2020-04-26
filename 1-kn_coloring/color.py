from copy import copy
import numpy as np
from tqdm import tqdm

N = 0

def comb(n, k):
    if k > n/2:
        k = n-k
    if k == 0:
        return 1
    if k < 0:
        return 0
    r = 1
    for i in range(k):
        r *= (n-i)
    for i in range(2, k+1):
        r /= i

    return int(r)

def dp_move(bc, color, cc, c, nxt, drop):
    if cc == 0:
        nxt[0] += 1
    
    else:
        masks = c.masks
        ci = 0
        cs = N
        for i, mask in enumerate(masks):
            cm = cc & mask
            if cm:
                if cs == N:
                    cs = bc[i]
                    ci = cm >> (i*c.blen)
                else:
                    return

        if cs == color:
            nxt[ci] += 1
        else:
            drop[ci] += 1
    

class C:
    def __init__(self, ncolors, maxn):
        self.blen = 0
        self.ncolors = ncolors
        while maxn:
            maxn = maxn >> 1
            self.blen += 1

        self.masks = []
        for i in range(ncolors):
            self.masks.append(((1<<self.blen)-1)<<(i*self.blen))

        self.bcolors = [1<<(i*self.blen) for i in range(ncolors)]

def cal_mono(n=100, k=4, ncolors=2):
    # kn中的边数
    ken = comb(k, 2)
    # 总边数
    en = comb(n, 2)
    # kn个数
    kn = comb(n, 4)
    # 编码颜色
    c = C(ncolors, ken)
    bc = c.bcolors
    # 预先计算常用的概率
    p = np.array([ncolors**(-i) for i in range(ken, 0, -1)])
    p[0] = p[1]
    # 处于不同染色状态的kn计数
    st = np.zeros((ken), dtype=int)
    # 边染色记录
    ec = np.zeros((n, n), dtype=int)

    # 初始染色
    ec[0][1] = bc[0]
    ec[1][0] = bc[0]
    st[1] = comb(n-2, 2)
    st[0] = kn - st[1]
    # 后续染色
    for i in tqdm(range(1, n)):
        for j in range(0, i):
            if ec[i][j] == N:
                lco = ec[i]+ec[j]
                ava_idx = np.argwhere(lco!=0).reshape(-1)
                ava_len = len(ava_idx) + sum([i not in ava_idx, j not in ava_idx])
                mc = [{'nxt':np.zeros((ken), dtype=int), 'drop': np.zeros((ken), dtype=int)} for i in range(ncolors)]
                n1c = comb(n-ava_len, 2)
                for m in mc:
                    m['nxt'][0] += n1c
                for q in ava_idx:
                    if q not in [i,j]:
                        for w in range(q+1, n):
                            if w not in [i,j]:
                                for color,m in zip(bc, mc):
                                    cc = lco[q] + lco[w] + ec[q][w]
                                    dp_move(bc, color, cc, c, m['nxt'], m['drop'])

                minp = np.inf
                #print(ec)
                #print(mc)
                #print(st)
                for q, m in enumerate(mc):
                    stc = st.copy()
                    stc -= m['nxt']+m['drop']
                    stc[1:] += m['nxt'][:-1]
                    prob = np.sum(stc*p)
                    if prob < minp:
                        minp = prob
                        bq = q
                ec[i][j] = bc[bq]
                ec[j][i] = bc[bq]
                st -= mc[bq]['nxt']+mc[bq]['drop']
                st[1:] += mc[bq]['nxt'][:-1]
                #print(st)

    print(ec)
    print(st)
    #np.save('ec4.npy', ec)

def check_kn(ec):
    n=100
    c = C(2, 6)
    kn = 0
    for i in tqdm(range(n-3)):
        for j in range(i+1,n-2):
            for q in range(j+1, n-1):
                for w in range(q+1, n):
                    al = ec[i][j] + ec[i,q]+ ec[i,w]+ ec[j,q]+ ec[j,w]+ ec[q,w]
                    if al == 6 or al == (6<<3):
                        kn += 1

    print(kn)

cal_mono(n=100, k=4, ncolors=2)

#ec = np.load('ec3.npy')
#check_kn(ec)

#c = C(2, 6)
#a=np.zeros((6), dtype=int)
#b=np.zeros((6), dtype=int)
#dp_move(c.bcolors, 1, 2, c, a, b)
#print(a,b)
                    
