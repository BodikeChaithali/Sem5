(define (domain cleaning-robot)
  (:requirements :strips :typing :negative-preconditions)

  (:types
    robot location
  )

  (:predicates
    (at ?r - robot ?l - location)
    (connected ?from ?to - location)
    (is-dustbin ?l - location)
    (dirty ?l - location)
    (swept ?l - location)
    (mopped ?l - location)
    (trash-at ?l - location)
    (carrying-trash ?r - robot)
  )

  ;; MOVE
  (:action move
    :parameters (?r - robot ?from ?to - location)
    :precondition (and (at ?r ?from)
                       (connected ?from ?to))
    :effect (and (at ?r ?to)
                 (not (at ?r ?from)))
  )

  ;; SWEEP
  (:action sweep
    :parameters (?r - robot ?l - location)
    :precondition (and (at ?r ?l)
                       (dirty ?l))
    :effect (and (swept ?l)
                 (not (dirty ?l)))
  )

  ;; MOP
  (:action mop
    :parameters (?r - robot ?l - location)
    :precondition (and (at ?r ?l)
                       (swept ?l))
    :effect (and (mopped ?l)
                 (not (swept ?l)))
  )

  ;; PICK-TRASH (only one unit at a time)
  (:action pick-trash
    :parameters (?r - robot ?l - location)
    :precondition (and (at ?r ?l)
                       (trash-at ?l)
                       (not (carrying-trash ?r)))
    :effect (and (carrying-trash ?r)
                 (not (trash-at ?l)))
  )

  ;; DROP-TRASH (only at the dustbin)
  (:action drop-trash
    :parameters (?r - robot ?d - location)
    :precondition (and (at ?r ?d)
                       (is-dustbin ?d)
                       (carrying-trash ?r))
    :effect (not (carrying-trash ?r))
  )
)
