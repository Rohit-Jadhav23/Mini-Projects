from flask import Flask, render_template,request, redirect,url_for,flash, session
from flask_mysqldb import MySQL
import MySQLdb.cursors
import bcrypt

app=Flask(__name__)
app.secret_key = 'Gabagool52'

app.config['MYSQL_HOST'] = 'localhost'
app.config['MYSQL_USER'] = 'root' 
app.config['MYSQL_PASSWORD'] = 'Root@1234' 
app.config['MYSQL_DB'] = 'iotdb'

mysql = MySQL(app)

@app.route('/about')
def about():
    return render_template('about.html')

@app.route('/')
@app.route('/blog')
def blog():
    cur = mysql.connection.cursor(MySQLdb.cursors.DictCursor)
    cur.execute("SELECT * FROM blogposts ORDER BY created_at DESC")
    posts = cur.fetchall()
    cur.close()
    
    return render_template('blog.html',posts=posts)

@app.route('/add',methods=['GET','POST'])
def addpost():
    if 'logged_in' not in session:
        flash('You must be logged in to add a post!', 'error')
        return redirect(url_for('login'))
    
    if request.method=='POST':
        title=request.form['title']
        author=request.form['author']
        content = request.form['content']

        cur = mysql.connection.cursor()
        cur.execute("INSERT INTO blogposts (title, author, content) VALUES (%s, %s, %s)", (title, author, content))
        mysql.connection.commit()
        cur.close()

        return redirect(url_for('blog'))  # Redirect back to blog page

    return render_template('add_post.html') 

@app.route('/delete/<int:id>')
def delete_post(id):
    if 'logged_in' not in session:
        flash('You must be logged in to delete posts!', 'error')
        return redirect(url_for('login'))
    
    cur = mysql.connection.cursor()
    cur.execute("DELETE FROM blogposts WHERE id = %s", [id])
    mysql.connection.commit()
    cur.close()
    
    return redirect(url_for('blog'))

@app.route('/edit/<int:id>', methods=['GET', 'POST'])
def edit_post(id):
    if 'logged_in' not in session:
        flash('You must be logged in to edit posts!', 'error')
        return redirect(url_for('login'))
    
    cur = mysql.connection.cursor(MySQLdb.cursors.DictCursor)
    
    if request.method == 'POST':
        title = request.form['title']
        author = request.form['author']
        content = request.form['content']
        
        cur.execute("UPDATE blogposts SET title=%s, author=%s, content=%s WHERE id=%s",
                   (title, author, content, id))
        mysql.connection.commit()
        cur.close()
        
        return redirect(url_for('blog'))
    
    cur.execute("SELECT * FROM blogposts WHERE id = %s", [id])
    post = cur.fetchone()
    cur.close()
    
    return render_template('edit_post.html', post=post)

@app.route('/logout')
def logout():
    session.clear()
    flash('Logged out successfully!', 'success')
    return redirect(url_for('blog'))

@app.route('/register',methods=['GET','POST'])
def register():
    if request.method == 'POST':
        username = request.form['username']
        email = request.form['email']
        password = request.form['password']
        
        # Hash the password
        hashed_password = bcrypt.hashpw(password.encode('utf-8'), bcrypt.gensalt())
        
        cur = mysql.connection.cursor()
        try:
            cur.execute("INSERT INTO blogusers (username, email, password) VALUES (%s, %s, %s)", (username, email, hashed_password))
            mysql.connection.commit()
            cur.close()
            
            flash('Registration successful! Please login.', 'success')
            return redirect(url_for('login'))
        except:
            flash('Username or email already exists!', 'error')
            cur.close()
    
    return render_template('register.html')

@app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        username = request.form['username']
        password = request.form['password']
        
        cur = mysql.connection.cursor(MySQLdb.cursors.DictCursor)
        cur.execute("SELECT * FROM blogusers WHERE username = %s", [username])
        user = cur.fetchone()
        cur.close()
        
        if user and bcrypt.checkpw(password.encode('utf-8'), user['password'].encode('utf-8')):
            # Login successful
            session['logged_in'] = True
            session['user_id'] = user['id']
            session['username'] = user['username']
            
            print("Login successful!")
            flash('Login successful!', 'success')
            return redirect(url_for('blog'))
        else:
            print("Login failed!")
            flash('Invalid username or password!', 'error')
    
    return render_template('login.html')

if __name__ == '__main__':
    app.run(debug=True)

