(ns jank.codegen.c++
  (:require [jank.codegen.sanitize :as sanitize]
            [jank.codegen.util :as util])
  (:use clojure.pprint
        jank.assert
        jank.debug.log))

; XXX: migrated
(defmulti codegen-impl
  (fn [current]
    (:kind current)))

; XXX: migrated
(defmethod codegen-impl :declare-statement
  [current]
  "")

; Only used for the main functions; all other functions
; are just local lambdas within main
; XXX: migrated
(defmethod codegen-impl :function-definition
  [current]
  (let [lambda (:value current)]
    (str (codegen-impl (:return lambda))
         " "
         (codegen-impl (:name current))
         (codegen-impl (:arguments lambda))
         "{"
         (util/reduce-spaced-map (comp util/end-statement codegen-impl)
                                 (:body lambda))
         "}")))

(defmethod codegen-impl :lambda-definition
  [current]
  (str "[=]"
       (codegen-impl (second current)) ; Params
       "->"
       (codegen-impl (nth current 2)) ; Return
       "{"
       (util/reduce-spaced-map (comp util/end-statement codegen-impl)
                               (drop 3 current))
       "}"))

(defmethod codegen-impl :binding-type
  [current]
  (let [value (nth current 2)]
    (cond
      ; Lambdas can be recursive, so their type needs to be specified
      (= (first value) :lambda-definition)
      (str "std::function<"
           (codegen-impl (nth value 2)) ; Return
           (codegen-impl (second value)) ; Params
           "> const ")

      ; Typically, we just want auto
      :else
      "auto const ")))

(defmethod codegen-impl :binding-name
  [current]
  (let [type (nth current 2)]
    (cond
      ; Lambda bindings contain type info in the name, to work around
      ; the lack of overloading in the target
      (= (first type) :lambda-definition)
      ;(second (second (util/serialize-binding-name current)))
      (second (second current)) ; TODO: mangling

      :else
      name)))

(defmethod codegen-impl :binding-definition
  [current]
  (str (codegen-impl (assoc (:type current) :kind :binding-type))
       (codegen-impl (update-in current [0] (fn [_] :binding-name))) ; Name
       "="
       (codegen-impl (nth current 2))))

(defmethod codegen-impl :function-call
  [current]
  (str ;(util/serialize-function-call ; TODO: mangling
         (codegen-impl (second current)) ; Name
         ;(nth current 3)) ; Signature
       "("
       (util/comma-separate-args
         (map codegen-impl (butlast (drop 2 current))))
       ")"))

; XXX: migrated
(defmethod codegen-impl :argument-list
  [current]
  (str "("
       (util/comma-separate-params
         (util/swap-params
           (map codegen-impl (:values current))))
       ")"))

; XXX: migrated
(defmethod codegen-impl :return-list
  [current]
  (if-let [ret (first (:values current))]
    (codegen-impl ret)
    "void"))

(defmethod codegen-impl :if-expression
  [current]
  (let [base (str "[=]()->"
                  ; If expressions used as returns need a type to be specified
                  (if (some #(and (vector? %) (= (first %) :type)) current)
                    (codegen-impl (second (nth current 4)))
                    "void")
                  "{if("
                  (codegen-impl (second (second current)))
                  "){"
                  (util/end-statement (codegen-impl (second (nth current 2))))
                  "}")]
    (str
      (cond
        (some #(and (vector? %) (= (first %) :else)) current)
        (str base
             "else{"
             (util/end-statement
               (codegen-impl (second (nth current 3))))
             "}")
        :else
        base)
      "}()")))

(defmethod codegen-impl :return
  [current]
  (str "return "
       (when (some? (second current))
         (codegen-impl (second current)))))

(defmethod codegen-impl :list
  [current]
  (str "("
       (util/reduce-spaced-map codegen-impl (rest current))
       ")"))

(defmethod codegen-impl :string
  [current]
  (str "\"" (second current) "\""))

(defmethod codegen-impl :integer
  [current]
  (second current))

(defmethod codegen-impl :real
  [current]
  (second current))

(defmethod codegen-impl :boolean
  [current]
  (second current))

(defmethod codegen-impl :identifier
  [current]
  ; Special case for function types
  (if (= "ƒ" (second current))
    (codegen-impl (update-in current [0] (fn [_] :function-type)))
    (str (apply str (mapcat (comp sanitize/sanitize str) (second current)))
         ; Handle generic specializations
         (when (= 3 (count current))
           (codegen-impl (nth current 2))))))

(defmethod codegen-impl :function-type
  [current]
  (str "std::function<"
       (let [return (second (nth (nth current 2) 2))]
         (if-not (nil? return)
           (codegen-impl return)
           "void"))
       "("
       (util/comma-separate-args
         (map codegen-impl (rest (second (nth current 2)))))
       ")>"))

(defmethod codegen-impl :type
  [current]
  (str (codegen-impl (second current)) " const"))

(defmethod codegen-impl :shortened-type
  [current]
  (str (codegen-impl (into [:identifier] current)) " const"))

(defmethod codegen-impl :specialization-list
  [current]
  (str "<" (codegen-impl (second current)) ">"))

(defmethod codegen-impl :default
  [current]
  (codegen-assert false (str "no codegen for '" current "'")))

(defn codegen [ast]
  (util/print-statement
    (codegen-impl
      {:kind :function-definition
       :name {:kind :identifier
              :name "#main"}
       :value {:kind :lambda-definition
               :arguments {:kind :argument-list
                           :values []}
               :return {:kind :return-list
                        :values []}
               :body (:cells ast)}})))
