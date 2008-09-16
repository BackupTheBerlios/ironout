;;; ironout.el --- a minor mode for using ironout
;; Copyright (C) 2008 Ali Gholami Rudi

;;; TODO
;; * handle file names with spaces in ironout-find
;; * interrupting ironout commands

(defgroup ironout nil
  "ironout, A C refactoring tool."
  :link '(url-link "http://ironout.berlios.de/")
  :prefix "ironout-")

(defcustom ironout-path "ironout"
  "The path to ironout executable.")

(defun ironout-rename ()
  "Rename occurrences of a name."
  (interactive)
  (let ((buffer (get-buffer-create "*ironout-patch*"))
	(newname (read-string "New Name: "))
	(filename (buffer-file-name)))
    (with-current-buffer buffer
      (setq buffer-read-only 'nil)
      (erase-buffer))
    (let ((offset (int-to-string (- (+ (point-min) (point)) 2))))
      (call-process ironout-path nil '("*ironout-patch*" nil) t
		    "rename" filename offset newname))
    (switch-to-buffer buffer)
    (goto-char (point-min))
    (diff-mode)
    (setq buffer-read-only 't)
  ))

(defvar ironout-current-hit 0
  "Points to current occurrence in *ironout-find* buffer.")

(defun ironout-find ()
  "Find occurrences of a name."
  (interactive)
  (let ((buffer (get-buffer-create "*ironout-find*")))
    (with-current-buffer buffer
      (setq buffer-read-only 'nil)
      (erase-buffer))
    (let ((offset (int-to-string (- (+ (point-min) (point)) 2))))
      (call-process ironout-path nil '("*ironout-find*" nil) t
		    "find" (buffer-file-name) offset))
    (with-current-buffer buffer
      (goto-char (point-min))
      (setq buffer-read-only 't)
      (local-set-key (kbd "RET") 'ironout-goto-hit)
      (local-set-key (kbd "C-j") 'ironout-goto-hit)
      (local-set-key (kbd "q") 'delete-window)
      (recenter))
    (setq ironout-current-hit 0)
    (setq next-error-function 'ironout-goto-next)
    (display-buffer buffer)
  ))

(defun ironout-goto-next (arg reset)
  (switch-to-buffer-other-window "*ironout-find*" t)
  (if reset
      (goto-char (point-min)))
  (setq ironout-current-hit (+ ironout-current-hit arg))
  (cond
   ((>= ironout-current-hit (line-number-at-pos (point-max)))
    (message "Moved past last hit!")
    (setq ironout-current-hit (line-number-at-pos (point-max))))
   ((<= ironout-current-hit 0)
    (message "Moved back before first hit!")
    (setq ironout-current-hit 0))
   ('t
    (goto-line ironout-current-hit)
    (ironout-goto-hit)))
  (if (string-equal "*ironout-find*" (buffer-name))
      (other-window -1)))

(defun ironout-goto-hit ()
  (interactive)
  (end-of-line)
  (let ((end (point)))
    (beginning-of-line)
    (let ((line (buffer-substring-no-properties (point) end)))
      (let ((tokens (split-string line)))
	(recenter 0)
	(find-file-other-window (nth 0 tokens))
	(goto-char (+ (string-to-number (nth 1 tokens)) 1))
	)
      )))

(defvar ironout-local-keymap (make-sparse-keymap))

(define-minor-mode ironout-mode
 "ironout, a C refactoring tool!" nil " Iron" ironout-local-keymap
  :global nil)

(define-key ironout-local-keymap (kbd "C-c i f") 'ironout-find)
(define-key ironout-local-keymap (kbd "C-c i r") 'ironout-rename)
(add-hook 'c-mode-hook 'ironout-mode)
(provide 'ironout)
