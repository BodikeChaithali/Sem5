(define (problem cleaning-problem)
  (:domain cleaning-robot)

  (:objects
    robot1 - robot
    classroom cafeteria dustbin - location
  )

  (:init
    ;; robot starts in classroom
    (at robot1 classroom)

    ;; layout: Classroom <-> Cafeteria <-> Dustbin
    (connected classroom cafeteria)
    (connected cafeteria classroom)
    (connected cafeteria dustbin)
    (connected dustbin cafeteria)

    (is-dustbin dustbin)

    ;; both rooms dirty
    (dirty classroom)
    (dirty cafeteria)

    ;; trash in both rooms
    (trash-at classroom)
    (trash-at cafeteria)
  )

  (:goal
    (and
      (mopped classroom)
      (mopped cafeteria)
      (not (trash-at classroom))     ;; all trash collected
      (not (trash-at cafeteria))
      (not (carrying-trash robot1))  ;; and disposed in dustbin
    )
  )
)
