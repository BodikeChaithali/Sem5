(define (domain mars-rover)
  (:requirements :strips :typing)
  (:types location rover bay rock drill)

  (:predicates
    (rover-at ?r - rover ?l - location)
    (rock-at ?k - rock ?l - location)
    (drill-at ?d - drill ?l - location)
    (connected ?from - location ?to - location)
    (soft ?k - rock)
    (hard ?k - rock)
    (has-drill ?r - rover ?d - drill)
    (bay-empty ?b - bay)
    (bay-holds ?b - bay ?k - rock)
  )

  (:action move
    :parameters (?r - rover ?from - location ?to - location)
    :precondition (and (rover-at ?r ?from) (connected ?from ?to))
    :effect (and (not (rover-at ?r ?from)) (rover-at ?r ?to))
  )

  (:action pick-drill
    :parameters (?r - rover ?d - drill ?l - location)
    :precondition (and (rover-at ?r ?l) (drill-at ?d ?l))
    :effect (and (not (drill-at ?d ?l)) (has-drill ?r ?d))
  )

  (:action collect-soft
    :parameters (?r - rover ?k - rock ?l - location ?b - bay)
    :precondition (and (rover-at ?r ?l) (rock-at ?k ?l) (soft ?k) (bay-empty ?b))
    :effect (and (not (rock-at ?k ?l)) (not (bay-empty ?b)) (bay-holds ?b ?k))
  )

  (:action collect-hard
    :parameters (?r - rover ?k - rock ?l - location ?b - bay ?d - drill)
    :precondition (and (rover-at ?r ?l) (rock-at ?k ?l) (hard ?k) (bay-empty ?b) (has-drill ?r ?d))
    :effect (and (not (rock-at ?k ?l)) (not (bay-empty ?b)) (bay-holds ?b ?k))
  )

  (:action deposit
    :parameters (?r - rover ?k - rock ?l - location ?b - bay)
    :precondition (and (rover-at ?r ?l) (bay-holds ?b ?k))
    :effect (and (not (bay-holds ?b ?k)) (bay-empty ?b) (rock-at ?k ?l))
  )
)
