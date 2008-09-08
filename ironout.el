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
      (call-process ironout-path nil buffer t
		    "rename" filename offset newname))
    (switch-to-buffer-other-window buffer)
    (goto-char (point-min))
    (diff-mode)
    (setq buffer-read-only 't)
  ))

(defun ironout-find ()
  "Find occurrences of a name."
  (interactive)
  (let ((buffer (get-buffer-create "*ironout-find*")))
    (with-current-buffer buffer
      (setq buffer-read-only 'nil)
      (erase-buffer))
    (call-process ironout-path
		  nil '("*ironout-find*" nil) t
		  "find" (buffer-file-name) (int-to-string
					     (- (+ (point-min) (point))
						2)))
    (with-current-buffer buffer
      (goto-char (point-min))
      (setq buffer-read-only 't)
      (local-set-key (kbd "RET") 'ironout-goto-occurrence)
      (local-set-key (kbd "C-j") 'ironout-goto-occurrence)
      (local-set-key (kbd "q") 'delete-window)
      (recenter))
    (setq next-error-function 'ironout-goto-next)
    (display-buffer buffer)
  ))

(defun ironout-goto-next (arg reset)
  (switch-to-buffer-other-window "*ironout-find*" t)
  (if reset
      (goto-char (point-min)))
  (forward-line (- arg 1))
  (if (eobp)
      ;; should go back to the older buffer here!
      (message "Moved past last occurrence!")
    (ironout-goto-occurrence))
  (if (string-equal "*ironout-find*" (buffer-name))
      (other-window -1)))

(defun ironout-goto-occurrence ()
  (interactive)
  (end-of-line)
  (let ((end (point)))
    (beginning-of-line)
    (let ((line (buffer-substring-no-properties (point) end)))
      (let ((tokens (split-string line)))
	(recenter 0)
	(forward-line 1)
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
