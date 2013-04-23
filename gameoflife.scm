(define @matrix-w)
(define @matrix-h)

(define (idx A i)
  (if (= 0 i)
      (car A)
      (idx (cdr A)
           (- i 1))))
;

(define (for-each A f)
  (if (null? A)
      #void
      (do (f (car A))
          (for-each (cdr A)
                    f))))
(define (reverse~ A B)
  (if (null? A)
      B
      (reverse~ (cdr A)
                (cons (car A) B))))
(define (reverse A)
  (reverse~ A '()))

(define (map~ A f B)
  (if (null? A)
      (reverse B)
      (map~ (cdr A)
            f
            (cons (f (car A))
                  B))))
(define (map f A)
  (map~ A f '()))
(define (repeat f times)
  (if (<= times 0)
      #void
      (do (f)
          (repeat f (- times 1)))))

(define (matrix-pos M x y)
  (if (or (< x 0)
          (< y 0)
          (>= x @matrix-w)
          (>= y @matrix-h))
      0
      (idx (idx M y) x)))
(define (matrix-map~r R x y A f)
  (if (null? R)
      (reverse A)
      (matrix-map~r (cdr R)
                    (+ x 1)
                    y
                    (cons (f (car R) x y)
                          A)
                    f)))
(define (matrix-map~ M y A f)
  (if (null? M)
      (reverse A)
      (matrix-map~ (cdr M)
                   (+ y 1)
                   (cons (matrix-map~r (car M) 0 y '() f)
                         A)
                   f)))
(define (matrix-map f M)
  (matrix-map~ M 0 '() f))

(define (matrix-tile q)
  (if (= q 0)
      ". "
      "[]"))
(define (matrix-display M)
  (for-each M (lambda (row)
                      (do (display (apply string-append (map matrix-tile row)))
                          (newline)))))
;
(define (neighbors M x y)
  (+ (matrix-pos M (- x 1) y)
     (matrix-pos M (- x 1) (- y 1))
     (matrix-pos M (- x 1) (+ y 1))
     (matrix-pos M (+ x 1) y)
     (matrix-pos M (+ x 1) (- y 1))
     (matrix-pos M (+ x 1) (+ y 1))
     (matrix-pos M x (+ y 1))
     (matrix-pos M x (- y 1))))


;

(define (populate-neighbors)
  (define *matrix-neighbors
          (matrix-map (lambda (t x y)
                        (neighbors *matrix-life x y))
                      *matrix-life)))
(define (update-life)
  (define *matrix-life
          (matrix-map (lambda (t x y)
                        (cond ((or (< t 2)
                                   (> t 3))
                               0)
                              ((= t 3)
                               1)
                              (else (matrix-pos *matrix-life x y))))
                      *matrix-neighbors)))
(define (display-life)
  (do (matrix-display *matrix-life)
      (newline)))
(define (life)
  (do (display-life)
      (populate-neighbors)
      (update-life)))



(define @matrix-w 10)
(define @matrix-h 8)

(define *matrix-life '((1 0 1 0 0 0 0 0 0 0)
                       (0 1 1 0 0 0 0 0 0 0)
                       (0 1 0 0 0 0 0 0 0 0)
                       (0 0 0 0 0 0 0 0 0 0)
                       (0 0 0 0 0 0 0 1 0 0)
                       (0 0 0 0 0 0 0 1 0 0)
                       (0 0 0 0 0 0 0 1 0 0)
                       (0 0 0 0 0 0 0 0 0 0)))
;
(repeat life 18)